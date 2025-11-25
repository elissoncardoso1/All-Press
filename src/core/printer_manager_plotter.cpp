#include "core/printer_manager.h"
#include "protocols/protocol_factory.h"
#include "protocols/compatibility_matrix.h"
#include "utils/logger.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace AllPress {

using namespace all_press::protocols;

// Detectar se é um plotter baseado no make_model
bool PrinterManager::is_plotter(const std::string& printer_uri) {
    auto info = get_printer_info(printer_uri);
    std::string model_lower = info.make_model;
    std::transform(model_lower.begin(), model_lower.end(), model_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    // Palavras-chave que identificam plotters
    std::vector<std::string> plotter_keywords = {
        "designjet", "imageprograf", "surecolor", 
        "plotter", "wide format", "large format"
    };
    
    for (const auto& keyword : plotter_keywords) {
        if (model_lower.find(keyword) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

// Detectar vendor de um plotter
PlotterVendor PrinterManager::detect_plotter_vendor(const std::string& make_model) {
    std::string model_lower = make_model;
    std::transform(model_lower.begin(), model_lower.end(), model_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    if (model_lower.find("hp") != std::string::npos || 
        model_lower.find("hewlett") != std::string::npos ||
        model_lower.find("designjet") != std::string::npos) {
        return PlotterVendor::HP;
    }
    else if (model_lower.find("canon") != std::string::npos ||
             model_lower.find("imageprograf") != std::string::npos) {
        return PlotterVendor::CANON;
    }
    else if (model_lower.find("epson") != std::string::npos ||
             model_lower.find("surecolor") != std::string::npos) {
        return PlotterVendor::EPSON;
    }
    
    return PlotterVendor::GENERIC;
}

// Descobrir plotters com informações avançadas
std::future<std::vector<PrinterManager::PrinterAdvancedInfo>> 
PrinterManager::discover_plotters_advanced_async() {
    return std::async(std::launch::async, [this]() {
        std::vector<PrinterAdvancedInfo> advanced_printers;
        
        // Descobrir todas as impressoras primeiro
        auto printers = discover_cups_printers();
        
        for (const auto& printer : printers) {
            if (is_plotter(printer.uri)) {
                PrinterAdvancedInfo adv_info;
                adv_info.base_info = printer;
                adv_info.vendor = detect_plotter_vendor(printer.make_model);
                
                // Obter protocolos suportados da matriz de compatibilidade
                std::string model = printer.make_model;
                adv_info.recommended_protocol = 
                    CompatibilityMatrix::get_recommended_protocol(adv_info.vendor, model);
                adv_info.supported_protocols = 
                    CompatibilityMatrix::get_fallback_protocols(adv_info.vendor, model);
                
                // Adicionar protocolo recomendado se não estiver na lista
                if (std::find(adv_info.supported_protocols.begin(),
                             adv_info.supported_protocols.end(),
                             adv_info.recommended_protocol) == 
                    adv_info.supported_protocols.end()) {
                    adv_info.supported_protocols.insert(
                        adv_info.supported_protocols.begin(),
                        adv_info.recommended_protocol);
                }
                
                // Criar protocolo handler para obter capabilities
                try {
                    auto protocol = PlotterProtocolFactory::create_for_printer(
                        adv_info.vendor, model);
                    adv_info.capabilities = protocol->get_capabilities();
                } catch (const std::exception& e) {
                    std::ostringstream oss;
                    oss << "Failed to create protocol for " << printer.name << ": " << e.what();
                    LOG_ERROR(oss.str());
                    // Usar capabilities padrão
                    adv_info.capabilities.vendor = adv_info.vendor;
                    adv_info.capabilities.model = model;
                }
                
                // Obter quirks conhecidos
                adv_info.manufacturer_specs = 
                    CompatibilityMatrix::get_quirks(adv_info.vendor, model);
                
                // Adicionar ao cache
                {
                    std::lock_guard<std::mutex> lock(plotter_cache_mutex_);
                    plotter_cache_[printer.uri] = adv_info;
                }
                
                advanced_printers.push_back(adv_info);
                
                std::ostringstream oss;
                oss << "Discovered plotter: " << printer.name << " (" << printer.make_model 
                    << "), Protocol: " << adv_info.recommended_protocol;
                LOG_INFO(oss.str());
            }
        }
        
        return advanced_printers;
    });
}

// Obter informações de plotter do cache ou descobrir
PrinterManager::PrinterAdvancedInfo PrinterManager::get_plotter_info(
    const std::string& printer_uri) {
    
    // Verificar cache primeiro
    {
        std::lock_guard<std::mutex> lock(plotter_cache_mutex_);
        auto it = plotter_cache_.find(printer_uri);
        if (it != plotter_cache_.end()) {
            return it->second;
        }
    }
    
    // Não está no cache, descobrir agora
    auto future = discover_plotters_advanced_async();
    auto plotters = future.get();
    
    for (const auto& plotter : plotters) {
        if (plotter.base_info.uri == printer_uri) {
            return plotter;
        }
    }
    
    // Retornar info vazia se não encontrado
    PrinterAdvancedInfo empty;
    empty.base_info = get_printer_info(printer_uri);
    empty.vendor = PlotterVendor::GENERIC;
    return empty;
}

// Selecionar melhor protocolo para um trabalho
std::string PrinterManager::select_best_protocol(
    const std::string& printer_uri,
    const PrintOptions& options) {
    
    auto plotter_info = get_plotter_info(printer_uri);
    
    // Se há um protocolo recomendado, usar esse
    if (!plotter_info.recommended_protocol.empty()) {
        std::ostringstream oss;
        oss << "Selected recommended protocol: " << plotter_info.recommended_protocol 
            << " for printer: " << printer_uri;
        LOG_INFO(oss.str());
        return plotter_info.recommended_protocol;
    }
    
    // Tentar selecionar baseado nas opções
    bool needs_color = (options.color_mode == "color");
    bool large_format = (options.media_size == "A0" || 
                        options.media_size == "A1" || 
                        options.media_size == "A2");
    
    // HP geralmente funciona melhor com HPGL2
    if (plotter_info.vendor == PlotterVendor::HP) {
        return "HPGL2";
    }
    // Canon e Epson preferem PostScript
    else if (plotter_info.vendor == PlotterVendor::CANON || 
             plotter_info.vendor == PlotterVendor::EPSON) {
        return "PostScript";
    }
    
    // Fallback padrão
    return "PostScript";
}

// Validar documento contra capacidades do plotter
bool PrinterManager::validate_document_for_plotter(
    const std::string& printer_uri,
    const std::string& file_path,
    const PrintOptions& options) {
    
    auto plotter_info = get_plotter_info(printer_uri);
    
    // Criar protocolo handler
    try {
        auto protocol = PlotterProtocolFactory::create_for_printer(
            plotter_info.vendor, plotter_info.base_info.make_model);
        
        // Converter string de media_size para enum
        MediaSize media_size = MediaSize::A4;  // Default
        if (options.media_size == "A0") media_size = MediaSize::A0;
        else if (options.media_size == "A1") media_size = MediaSize::A1;
        else if (options.media_size == "A2") media_size = MediaSize::A2;
        else if (options.media_size == "A3") media_size = MediaSize::A3;
        else if (options.media_size == "A4") media_size = MediaSize::A4;
        else if (options.media_size == "Letter") media_size = MediaSize::LETTER;
        else if (options.media_size == "Legal") media_size = MediaSize::LEGAL;
        
        // Validar media size
        if (!protocol->validate_media_size(media_size)) {
            std::ostringstream oss;
            oss << "Media size " << options.media_size << " not supported by plotter " << printer_uri;
            LOG_ERROR(oss.str());
            return false;
        }
        
        // Converter color_mode para enum
        ColorMode color_mode = ColorMode::MONOCHROME;
        if (options.color_mode == "color") {
            color_mode = ColorMode::COLOR;
        }
        
        // Validar color mode
        if (!protocol->validate_color_mode(color_mode)) {
            std::ostringstream oss;
            oss << "Color mode " << options.color_mode << " not supported by plotter " << printer_uri;
            LOG_ERROR(oss.str());
            return false;
        }
        
        // Validar resolução (usar qualidade como proxy para DPI)
        int dpi = 300;
        if (options.quality == 5) dpi = 1200;
        else if (options.quality == 4) dpi = 600;
        else if (options.quality == 3) dpi = 600;
        else if (options.quality == 2) dpi = 300;
        else if (options.quality == 1) dpi = 300;
        
        if (!protocol->validate_resolution(dpi)) {
            std::ostringstream oss;
            oss << "Resolution " << dpi << " DPI not optimal for plotter " << printer_uri << ", will use closest";
            LOG_WARNING(oss.str());
        }
        
        std::ostringstream oss;
        oss << "Document validation passed for plotter: " << printer_uri;
        LOG_INFO(oss.str());
        return true;
        
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Failed to validate document: " << e.what();
        LOG_ERROR(oss.str());
        return false;
    }
}

} // namespace AllPress


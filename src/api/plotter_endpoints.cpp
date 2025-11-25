#include "api/rest_server.h"
#include "protocols/protocol_factory.h"
#include "protocols/compatibility_matrix.h"
#include "utils/logger.h"
#include <sstream>
#include <nlohmann/json.hpp>

namespace AllPress {
namespace API {

using json = nlohmann::json;
using namespace all_press::protocols;

// Converter PlotterVendor para string
std::string vendor_to_string(PlotterVendor vendor) {
    switch (vendor) {
        case PlotterVendor::HP: return "HP";
        case PlotterVendor::CANON: return "Canon";
        case PlotterVendor::EPSON: return "Epson";
        case PlotterVendor::GENERIC: return "Generic";
        default: return "Unknown";
    }
}

// Converter MediaSize para string
std::string media_size_to_string(MediaSize size) {
    switch (size) {
        case MediaSize::A0: return "A0";
        case MediaSize::A1: return "A1";
        case MediaSize::A2: return "A2";
        case MediaSize::A3: return "A3";
        case MediaSize::A4: return "A4";
        case MediaSize::A5: return "A5";
        case MediaSize::LETTER: return "Letter";
        case MediaSize::LEGAL: return "Legal";
        case MediaSize::TABLOID: return "Tabloid";
        default: return "Custom";
    }
}

// Converter ColorMode para string
std::string color_mode_to_string(ColorMode mode) {
    switch (mode) {
        case ColorMode::MONOCHROME: return "monochrome";
        case ColorMode::COLOR: return "color";
        case ColorMode::RGB: return "rgb";
        case ColorMode::CMYK: return "cmyk";
        default: return "unknown";
    }
}

// Handler: GET /api/plotters/discover
std::string handle_discover_plotters(PrinterManager* printer_mgr) {
    LOG_INFO("API: Discovering plotters");
    
    try {
        auto future = printer_mgr->discover_plotters_advanced_async();
        auto plotters = future.get();
        
        json response = json::array();
        
        for (const auto& plotter : plotters) {
            json plotter_json;
            plotter_json["name"] = plotter.base_info.name;
            plotter_json["uri"] = plotter.base_info.uri;
            plotter_json["make_model"] = plotter.base_info.make_model;
            plotter_json["location"] = plotter.base_info.location;
            plotter_json["vendor"] = vendor_to_string(plotter.vendor);
            plotter_json["recommended_protocol"] = plotter.recommended_protocol;
            plotter_json["supported_protocols"] = plotter.supported_protocols;
            plotter_json["is_online"] = plotter.base_info.is_online;
            
            response.push_back(plotter_json);
        }
        
        return response.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Failed to discover plotters";
        error["message"] = e.what();
        return error.dump(2);
    }
}

// Handler: GET /api/plotters/{id}/capabilities
std::string handle_plotter_capabilities(PrinterManager* printer_mgr, const std::string& printer_uri) {
    std::string log_msg = "API: Getting capabilities for plotter: " + printer_uri;
    LOG_INFO(log_msg);
    
    try {
        auto plotter_info = printer_mgr->get_plotter_info(printer_uri);
        
        json response;
        response["name"] = plotter_info.base_info.name;
        response["vendor"] = vendor_to_string(plotter_info.vendor);
        response["model"] = plotter_info.capabilities.model;
        
        // Supported sizes
        json sizes = json::array();
        for (const auto& size : plotter_info.capabilities.supported_sizes) {
            sizes.push_back(media_size_to_string(size));
        }
        response["supported_sizes"] = sizes;
        
        // Supported resolutions
        response["supported_resolutions"] = plotter_info.capabilities.supported_resolutions;
        
        // Supported color modes
        json colors = json::array();
        for (const auto& color : plotter_info.capabilities.supported_colors) {
            colors.push_back(color_mode_to_string(color));
        }
        response["supported_colors"] = colors;
        
        // Capabilities
        response["supports_duplex"] = plotter_info.capabilities.supports_duplex;
        response["supports_booklet"] = plotter_info.capabilities.supports_booklet;
        response["max_paper_width_mm"] = plotter_info.capabilities.max_paper_width_mm;
        response["max_paper_height_mm"] = plotter_info.capabilities.max_paper_height_mm;
        
        // Custom attributes
        response["custom_attributes"] = plotter_info.capabilities.custom_attributes;
        
        return response.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Failed to get plotter capabilities";
        error["message"] = e.what();
        return error.dump(2);
    }
}

// Handler: GET /api/plotters/{id}/protocols
std::string handle_plotter_protocols(PrinterManager* printer_mgr, const std::string& printer_uri) {
    std::string log_msg = "API: Getting protocols for plotter: " + printer_uri;
    LOG_INFO(log_msg);
    
    try {
        auto plotter_info = printer_mgr->get_plotter_info(printer_uri);
        
        json response;
        response["printer"] = plotter_info.base_info.name;
        response["recommended_protocol"] = plotter_info.recommended_protocol;
        response["supported_protocols"] = plotter_info.supported_protocols;
        
        // Detalhes de cada protocolo
        json protocols_detail = json::array();
        for (const auto& protocol_name : plotter_info.supported_protocols) {
            json protocol_info;
            protocol_info["name"] = protocol_name;
            protocol_info["is_recommended"] = (protocol_name == plotter_info.recommended_protocol);
            
            // Adicionar informações específicas do protocolo
            if (protocol_name == "HPGL2" || protocol_name == "HPGL") {
                protocol_info["requires_preprocessing"] = true;
                protocol_info["description"] = "Hewlett-Packard Graphics Language";
            } else if (protocol_name == "PostScript") {
                protocol_info["requires_preprocessing"] = false;
                protocol_info["description"] = "Adobe PostScript Level 3";
            } else if (protocol_name == "ESC/P") {
                protocol_info["requires_preprocessing"] = false;
                protocol_info["description"] = "Epson ESC/P Command Set";
            }
            
            protocols_detail.push_back(protocol_info);
        }
        response["protocols"] = protocols_detail;
        
        return response.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Failed to get plotter protocols";
        error["message"] = e.what();
        return error.dump(2);
    }
}

// Handler: POST /api/plotters/{id}/validate
std::string handle_validate_document(
    PrinterManager* printer_mgr,
    const std::string& printer_uri,
    const std::string& request_body) {
    
    std::string log_msg = "API: Validating document for plotter: " + printer_uri;
    LOG_INFO(log_msg);
    
    try {
        auto request = json::parse(request_body);
        
        // Extrair opções
        PrintOptions options;
        if (request.contains("media_size")) {
            options.media_size = request["media_size"].get<std::string>();
        }
        if (request.contains("color_mode")) {
            options.color_mode = request["color_mode"].get<std::string>();
        }
        if (request.contains("resolution")) {
            int dpi = request["resolution"].get<int>();
            // Converter DPI para quality (1-5)
            if (dpi >= 1200) options.quality = 5;
            else if (dpi >= 600) options.quality = 4;
            else options.quality = 3;
        }
        
        std::string file_path = request["file_path"].get<std::string>();
        
        // Validar
        bool is_valid = printer_mgr->validate_document_for_plotter(
            printer_uri, file_path, options);
        
        json response;
        response["valid"] = is_valid;
        response["printer"] = printer_uri;
        response["file"] = file_path;
        
        if (is_valid) {
            response["message"] = "Document is compatible with plotter";
            
            // Selecionar melhor protocolo
            std::string protocol = printer_mgr->select_best_protocol(
                printer_uri, options);
            response["recommended_protocol"] = protocol;
        } else {
            response["message"] = "Document is not compatible with plotter";
            response["reason"] = "Media size or color mode not supported";
        }
        
        return response.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Failed to validate document";
        error["message"] = e.what();
        return error.dump(2);
    }
}

// Handler: POST /api/print/auto
std::string handle_auto_print(
    PrinterManager* printer_mgr,
    JobQueue* job_queue,
    const std::string& request_body) {
    
    LOG_INFO("API: Auto print with protocol selection");
    
    try {
        auto request = json::parse(request_body);
        
        std::string printer = request["printer"].get<std::string>();
        std::string file_path = request["file_path"].get<std::string>();
        
        // Extrair opções
        PrintOptions options;
        if (request.contains("options")) {
            auto opts = request["options"];
            if (opts.contains("media_size")) {
                options.media_size = opts["media_size"].get<std::string>();
            }
            if (opts.contains("color_mode")) {
                options.color_mode = opts["color_mode"].get<std::string>();
            }
            if (opts.contains("copies")) {
                options.copies = opts["copies"].get<int>();
            }
            if (opts.contains("quality")) {
                options.quality = opts["quality"].get<int>();
            }
        }
        
        // Selecionar protocolo automaticamente
        std::string protocol = printer_mgr->select_best_protocol(printer, options);
        
        // Criar job
        PrintJob job;
        job.printer_name = printer;
        job.file_path = file_path;
        job.original_filename = file_path.substr(file_path.find_last_of("/\\") + 1);
        job.options = options;
        job.created_at = std::chrono::system_clock::now();
        
        // Adicionar à fila
        int job_id = job_queue->add_job(job);
        
        json response;
        response["job_id"] = job_id;
        response["printer"] = printer;
        response["file"] = file_path;
        response["selected_protocol"] = protocol;
        response["status"] = "queued";
        response["message"] = "Job submitted successfully";
        
        return response.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Failed to submit job";
        error["message"] = e.what();
        return error.dump(2);
    }
}

// Handler: GET /api/compatibility-matrix
std::string handle_compatibility_matrix() {
    LOG_INFO("API: Getting compatibility matrix");
    
    try {
        auto all_plotters = CompatibilityMatrix::get_all_plotters();
        
        json response = json::array();
        
        for (const auto& info : all_plotters) {
            json plotter_json;
            plotter_json["vendor"] = vendor_to_string(info.vendor);
            plotter_json["model"] = info.model;
            plotter_json["supported_protocols"] = info.supported_protocols;
            plotter_json["primary_protocol"] = info.primary_protocol;
            plotter_json["fallback_protocols"] = info.fallback_protocols;
            plotter_json["requires_preprocessing"] = info.requires_preprocessing;
            plotter_json["quirks"] = info.quirks;
            
            response.push_back(plotter_json);
        }
        
        return response.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Failed to get compatibility matrix";
        error["message"] = e.what();
        return error.dump(2);
    }
}

// Handler: GET /api/plotters/{vendor}/{model}/quirks
std::string handle_plotter_quirks(const std::string& vendor_str, const std::string& model) {
    std::string log_msg = "API: Getting quirks for " + vendor_str + " " + model;
    LOG_INFO(log_msg);
    
    try {
        // Converter string para enum
        PlotterVendor vendor = PlotterVendor::GENERIC;
        if (vendor_str == "hp" || vendor_str == "HP") {
            vendor = PlotterVendor::HP;
        } else if (vendor_str == "canon" || vendor_str == "Canon") {
            vendor = PlotterVendor::CANON;
        } else if (vendor_str == "epson" || vendor_str == "Epson") {
            vendor = PlotterVendor::EPSON;
        }
        
        auto quirks = CompatibilityMatrix::get_quirks(vendor, model);
        
        json response;
        response["vendor"] = vendor_str;
        response["model"] = model;
        response["quirks"] = quirks;
        
        if (quirks.empty()) {
            response["message"] = "No known quirks for this plotter";
        }
        
        return response.dump(2);
        
    } catch (const std::exception& e) {
        json error;
        error["error"] = "Failed to get quirks";
        error["message"] = e.what();
        return error.dump(2);
    }
}

} // namespace API
} // namespace AllPress


#pragma once

#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <future>
#include <chrono>
#include <functional>
#include <atomic>
#include <thread>
#include "protocols/plotter_protocol_base.h"

#ifdef __APPLE__
#include <cups/cups.h>
#elif __linux__
#include <cups/cups.h>
#endif

namespace AllPress {

struct PrinterInfo {
    std::string name;
    std::string uri;
    std::string make_model;
    std::string location;
    std::string description;
    int status;
    int jobs_count;
    bool is_online;
    std::chrono::system_clock::time_point last_updated;
};

struct PrintOptions {
    std::string media_size = "A4";
    std::string color_mode = "color";
    std::string duplex = "none";
    int copies = 1;
    int quality = 3; // 1-5 scale
    std::string orientation = "portrait";
    bool collate = true;
};

class PrinterManager {
public:
    PrinterManager();
    ~PrinterManager();
    
    // Descoberta assíncrona de impressoras
    std::future<std::vector<PrinterInfo>> discover_printers_async();
    std::vector<PrinterInfo> discover_cups_printers();
    std::vector<PrinterInfo> discover_ipp_printers(const std::string& subnet = "192.168.1");
    
    // Gerenciamento
    bool add_printer(const std::string& name, const std::string& uri, 
                    const std::string& driver = "");
    bool remove_printer(const std::string& name);
    PrinterInfo get_printer_info(const std::string& name);
    std::vector<PrinterInfo> get_all_printers();
    
    // Status em tempo real
    void start_status_monitoring();
    void stop_status_monitoring();
    void register_status_callback(std::function<void(const PrinterInfo&)> callback);
    
    // Impressão
    int submit_print_job(const std::string& printer, const std::string& file_path,
                        const PrintOptions& options);
    bool cancel_job(int job_id);
    bool pause_job(int job_id);
    bool resume_job(int job_id);
    
    // Capacidades
    std::vector<std::string> get_supported_media_sizes(const std::string& printer);
    std::vector<std::string> get_supported_color_modes(const std::string& printer);
    bool supports_duplex(const std::string& printer);
    
    // 🆕 Suporte avançado a Plotters
    struct PrinterAdvancedInfo {
        PrinterInfo base_info;
        all_press::protocols::PlotterVendor vendor;
        std::vector<std::string> supported_protocols;
        std::string recommended_protocol;
        all_press::protocols::PlotterCapabilities capabilities;
        std::map<std::string, std::string> manufacturer_specs;
    };
    
    // Descobrir impressoras com informações avançadas
    std::future<std::vector<PrinterAdvancedInfo>> discover_plotters_advanced_async();
    
    // Validar documento contra capacidades do plotter
    bool validate_document_for_plotter(
        const std::string& printer_uri,
        const std::string& file_path,
        const PrintOptions& options);
    
    // Obter informações de compatibilidade
    PrinterAdvancedInfo get_plotter_info(const std::string& printer_uri);
    
    // Selecionar protocolo automaticamente
    std::string select_best_protocol(
        const std::string& printer_uri,
        const PrintOptions& options);
    
    // Detectar se é um plotter
    bool is_plotter(const std::string& printer_uri);
    
    // Extrair vendor de um modelo
    all_press::protocols::PlotterVendor detect_plotter_vendor(const std::string& make_model);

private:
    void update_printer_status();
    void monitor_thread_func();
    
    std::vector<PrinterInfo> printers_;
    std::mutex printers_mutex_;
    std::thread monitor_thread_;
    std::atomic<bool> monitoring_active_{false};
    std::vector<std::function<void(const PrinterInfo&)>> status_callbacks_;
    
    // 🆕 Cache de informações de plotters
    std::map<std::string, PrinterAdvancedInfo> plotter_cache_;
    std::mutex plotter_cache_mutex_;
    
#if defined(__APPLE__) || defined(__linux__)
    cups_dest_t* cups_dests_ = nullptr;
    int num_cups_dests_ = 0;
#endif
};

} // namespace AllPress

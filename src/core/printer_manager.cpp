#include "core/printer_manager.h"
#include "utils/logger.h"
#include <algorithm>
#include <sstream>

#ifdef __APPLE__
#include <cups/cups.h>
#elif __linux__
#include <cups/cups.h>
#endif

namespace AllPress {

PrinterManager::PrinterManager() {
    LOG_INFO("PrinterManager initialized");
}

PrinterManager::~PrinterManager() {
    stop_status_monitoring();
#if defined(__APPLE__) || defined(__linux__)
    if (cups_dests_) {
        cupsFreeDests(num_cups_dests_, cups_dests_);
    }
#endif
}

std::future<std::vector<PrinterInfo>> PrinterManager::discover_printers_async() {
    return std::async(std::launch::async, [this]() {
        return discover_cups_printers();
    });
}

std::vector<PrinterInfo> PrinterManager::discover_cups_printers() {
    std::vector<PrinterInfo> printers;
    
#if defined(__APPLE__) || defined(__linux__)
    if (cups_dests_) {
        cupsFreeDests(num_cups_dests_, cups_dests_);
    }
    
    num_cups_dests_ = cupsGetDests(&cups_dests_);
    
    for (int i = 0; i < num_cups_dests_; i++) {
        cups_dest_t* dest = &cups_dests_[i];
        
        PrinterInfo info;
        info.name = dest->name;
        info.uri = "cups://" + std::string(dest->name);
        info.jobs_count = 0;
        info.last_updated = std::chrono::system_clock::now();
        
        // Get printer description
        const char* make_model = cupsGetOption("printer-make-and-model", 
                                              dest->num_options, 
                                              dest->options);
        if (make_model) {
            info.make_model = make_model;
        }
        
        const char* location = cupsGetOption("printer-location", 
                                            dest->num_options, 
                                            dest->options);
        if (location) {
            info.location = location;
        }
        
        const char* description = cupsGetOption("printer-info", 
                                               dest->num_options, 
                                               dest->options);
        if (description) {
            info.description = description;
        }
        
        // Obter o URI real da impressora (não o CUPS local)
        const char* device_uri = cupsGetOption("device-uri", dest->num_options, dest->options);
        std::string real_uri;
        if (device_uri) {
            real_uri = device_uri;
            info.uri = device_uri; // Usar URI real, não CUPS local
        } else {
            real_uri = "cups://" + std::string(dest->name);
        }
        
        // Por padrão, considerar offline até provar o contrário
        info.is_online = false;
        info.status = 5; // stopped
        
        // Primeiro: verificar estado local do CUPS
        ipp_t* request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
        std::string cups_uri = "ipp://localhost/printers/" + std::string(dest->name);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, cups_uri.c_str());
        
        bool cups_says_ready = false;
        ipp_t* response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");
        
        if (response) {
            ipp_attribute_t* attr = ippFindAttribute(response, "printer-state", IPP_TAG_ENUM);
            if (attr) {
                int state = ippGetInteger(attr, 0);
                info.status = state;
                cups_says_ready = (state == 3 || state == 4);
            }
            
            // Verificar razões de estado
            attr = ippFindAttribute(response, "printer-state-reasons", IPP_TAG_KEYWORD);
            if (attr) {
                for (int j = 0; j < ippGetCount(attr); j++) {
                    const char* reason = ippGetString(attr, j, nullptr);
                    if (reason) {
                        std::string reason_str(reason);
                        if (reason_str.find("offline") != std::string::npos ||
                            reason_str.find("shutdown") != std::string::npos ||
                            reason_str.find("paused") != std::string::npos) {
                            cups_says_ready = false;
                        }
                    }
                }
            }
            
            ippDelete(response);
        }
        
        // Segundo: SE o CUPS diz que está pronta, verificar conectividade REAL
        if (cups_says_ready && !real_uri.empty()) {
            // Tentar conexão real com a impressora (timeout de 2 segundos)
            http_t* http_conn = nullptr;
            
            // Extrair host e porta do URI
            if (real_uri.find("ipp://") == 0 || real_uri.find("http://") == 0 || 
                real_uri.find("socket://") == 0 || real_uri.find("lpd://") == 0) {
                
                std::string host;
                int port = 631; // porta IPP padrão
                
                // Parse básico do URI para extrair host
                size_t proto_end = real_uri.find("://");
                if (proto_end != std::string::npos) {
                    size_t host_start = proto_end + 3;
                    size_t host_end = real_uri.find("/", host_start);
                    size_t port_pos = real_uri.find(":", host_start);
                    
                    if (port_pos != std::string::npos && port_pos < host_end) {
                        host = real_uri.substr(host_start, port_pos - host_start);
                        std::string port_str = real_uri.substr(port_pos + 1, host_end - port_pos - 1);
                        port = std::stoi(port_str);
                    } else if (host_end != std::string::npos) {
                        host = real_uri.substr(host_start, host_end - host_start);
                    } else {
                        host = real_uri.substr(host_start);
                    }
                }
                
                // Tentar conectar com timeout curto (2 segundos)
                if (!host.empty() && host != "localhost" && host != "127.0.0.1") {
                    http_conn = httpConnect2(host.c_str(), port, nullptr, AF_UNSPEC, 
                                            HTTP_ENCRYPTION_IF_REQUESTED, 1, 2000, nullptr);
                    
                    if (http_conn) {
                        // Conseguiu conectar - impressora está realmente online
                        info.is_online = true;
                        httpClose(http_conn);
                    } else {
                        // Falhou ao conectar - impressora offline na rede
                        info.is_online = false;
                        info.status = 5;
                    }
                } else {
                    // Impressora local (USB, etc) - confiar no CUPS
                    info.is_online = cups_says_ready;
                }
            } else {
                // URI não-rede (USB, file, etc) - confiar no CUPS
                info.is_online = cups_says_ready;
            }
        }
        
        // Log do status para debug
        std::ostringstream log_msg;
        log_msg << "Printer: " << info.name 
                << " | URI: " << real_uri
                << " | CUPS Status: " << info.status 
                << " | CUPS Ready: " << (cups_says_ready ? "YES" : "NO")
                << " | Network Online: " << (info.is_online ? "YES" : "NO");
        LOG_INFO(log_msg.str());
        
        printers.push_back(info);
    }
    
    std::lock_guard<std::mutex> lock(printers_mutex_);
    printers_ = printers;
#endif
    
    LOG_INFO("Discovered " + std::to_string(printers.size()) + " CUPS printers");
    return printers;
}

std::vector<PrinterInfo> PrinterManager::discover_ipp_printers(const std::string& subnet) {
    std::vector<PrinterInfo> printers;
    LOG_INFO("IPP discovery not yet implemented for subnet: " + subnet);
    return printers;
}

bool PrinterManager::add_printer(const std::string& name, const std::string& uri, 
                                const std::string& driver) {
    LOG_INFO("Adding printer: " + name + " at " + uri);
    // Implementation would use CUPS API
    return true;
}

bool PrinterManager::remove_printer(const std::string& name) {
    LOG_INFO("Removing printer: " + name);
    return true;
}

PrinterInfo PrinterManager::get_printer_info(const std::string& name) {
    std::lock_guard<std::mutex> lock(printers_mutex_);
    auto it = std::find_if(printers_.begin(), printers_.end(),
        [&name](const PrinterInfo& p) { return p.name == name; });
    
    if (it != printers_.end()) {
        return *it;
    }
    
    return PrinterInfo();
}

std::vector<PrinterInfo> PrinterManager::get_all_printers() {
    std::lock_guard<std::mutex> lock(printers_mutex_);
    return printers_;
}

void PrinterManager::start_status_monitoring() {
    if (!monitoring_active_.exchange(true)) {
        monitor_thread_ = std::thread(&PrinterManager::monitor_thread_func, this);
        LOG_INFO("Status monitoring started");
    }
}

void PrinterManager::stop_status_monitoring() {
    if (monitoring_active_.exchange(false)) {
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
        LOG_INFO("Status monitoring stopped");
    }
}

void PrinterManager::register_status_callback(std::function<void(const PrinterInfo&)> callback) {
    status_callbacks_.push_back(callback);
}

int PrinterManager::submit_print_job(const std::string& printer, const std::string& file_path,
                                    const PrintOptions& options) {
#if defined(__APPLE__) || defined(__linux__)
    LOG_INFO("Attempting to print file: " + file_path + " to printer: " + printer);
    
    cups_dest_t* dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer.c_str(), nullptr);
    if (!dest) {
        LOG_ERROR("Printer not found: " + printer);
        LOG_ERROR("CUPS error: " + std::string(cupsLastErrorString()));
        return -1;
    }
    
    LOG_INFO("Printer found: " + printer);
    
    int num_options = 0;
    cups_option_t* cup_options = nullptr;
    
    // Set print options
    num_options = cupsAddOption("media", options.media_size.c_str(), num_options, &cup_options);
    num_options = cupsAddOption("copies", std::to_string(options.copies).c_str(), num_options, &cup_options);
    
    if (options.color_mode == "color") {
        num_options = cupsAddOption("print-color-mode", "color", num_options, &cup_options);
    } else {
        num_options = cupsAddOption("print-color-mode", "monochrome", num_options, &cup_options);
    }
    
    LOG_INFO("Calling cupsPrintFile with printer=" + printer + ", file=" + file_path);
    
    int job_id = cupsPrintFile(printer.c_str(), file_path.c_str(), 
                               "AllPress Job", num_options, cup_options);
    
    cupsFreeOptions(num_options, cup_options);
    cupsFreeDests(1, dest);
    
    if (job_id > 0) {
        LOG_INFO("Print job submitted successfully with job ID: " + std::to_string(job_id));
    } else {
        std::string cups_error = cupsLastErrorString();
        LOG_ERROR("Failed to submit print job. CUPS error: " + cups_error);
        LOG_ERROR("CUPS error code: " + std::to_string(cupsLastError()));
    }
    
    return job_id;
#else
    LOG_ERROR("CUPS not supported on this platform");
    return -1;
#endif
}

bool PrinterManager::cancel_job(int job_id) {
#if defined(__APPLE__) || defined(__linux__)
    int result = cupsCancelJob(nullptr, job_id);
    LOG_INFO("Cancel job " + std::to_string(job_id) + ": " + 
             (result ? "success" : "failed"));
    return result == 1;
#else
    return false;
#endif
}

bool PrinterManager::pause_job(int job_id) {
    LOG_INFO("Pause job: " + std::to_string(job_id));
    return true;
}

bool PrinterManager::resume_job(int job_id) {
    LOG_INFO("Resume job: " + std::to_string(job_id));
    return true;
}

std::vector<std::string> PrinterManager::get_supported_media_sizes(const std::string& printer) {
    return {"A4", "A3", "Letter", "Legal", "Tabloid"};
}

std::vector<std::string> PrinterManager::get_supported_color_modes(const std::string& printer) {
    return {"color", "monochrome", "grayscale"};
}

bool PrinterManager::supports_duplex(const std::string& printer) {
    return true;
}

void PrinterManager::update_printer_status() {
    // Update printer status from CUPS
    discover_cups_printers();
    
    // Notify callbacks
    std::lock_guard<std::mutex> lock(printers_mutex_);
    for (const auto& printer : printers_) {
        for (const auto& callback : status_callbacks_) {
            callback(printer);
        }
    }
}

void PrinterManager::monitor_thread_func() {
    while (monitoring_active_) {
        update_printer_status();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

} // namespace AllPress

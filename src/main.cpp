#include <iostream>
#include <signal.h>
#include <atomic>

#include "core/printer_manager.h"
#include "core/job_queue.h"
#include "core/color_manager.h"
#include "network/ipp_client.h"
#include "network/network_scanner.h"
#include "conversion/file_processor.h"
#include "database/sqlite_manager.h"
#include "api/rest_server.h"
#include "api/websocket_server.h"
#include "utils/logger.h"
#include "utils/config.h"

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        LOG_INFO("Shutdown signal received");
        running = false;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "===========================================\n";
    std::cout << "   All Press C++ - High-Performance Print\n";
    std::cout << "   Management System v1.1.0\n";
    std::cout << "===========================================\n\n";
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize logger
    AllPress::Utils::Logger::instance().set_level(AllPress::Utils::LogLevel::INFO);
    AllPress::Utils::Logger::instance().enable_console_output(true);
    AllPress::Utils::Logger::instance().set_output_file("all_press.log");
    
    LOG_INFO("Starting All Press C++ Server...");
    
    // Load configuration
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    if (!config.load_from_file("config/all_press.conf")) {
        LOG_WARNING("Could not load config file, using defaults");
    }
    
    int port = config.get_int("server.port", 8000);
    int ws_port = config.get_int("server.ws_port", 8001);
    int max_workers = config.get_int("queue.max_workers", 4);
    
    try {
        // Initialize database
        LOG_INFO("Initializing database...");
        AllPress::Database::SQLiteManager db("all_press.db");
        if (!db.initialize()) {
            LOG_ERROR("Failed to initialize database");
            return 1;
        }
        
        // Initialize color manager
        LOG_INFO("Initializing color management...");
        AllPress::Color::ColorManager color_manager;
        color_manager.initialize();
        
        // Initialize printer manager
        LOG_INFO("Initializing printer manager...");
        AllPress::PrinterManager printer_manager;
        
        // Discover printers
        LOG_INFO("Discovering printers...");
        auto printers = printer_manager.discover_cups_printers();
        LOG_INFO("Found " + std::to_string(printers.size()) + " printers");
        
        for (const auto& printer : printers) {
            std::cout << "  - " << printer.name << " (" << printer.uri << ")\n";
        }
        
        // Initialize job queue
        LOG_INFO("Initializing job queue with " + std::to_string(max_workers) + " workers...");
        AllPress::JobQueue job_queue(max_workers);
        job_queue.set_printer_manager(&printer_manager);
        job_queue.start();
        
        // Initialize file processor
        LOG_INFO("Initializing file processor...");
        AllPress::FileProcessor file_processor;
        
        // Start printer status monitoring
        LOG_INFO("Starting printer status monitoring...");
        printer_manager.start_status_monitoring();
        
        // Register status callback
        printer_manager.register_status_callback([](const AllPress::PrinterInfo& info) {
            LOG_DEBUG("Printer status update: " + info.name + " - " + 
                     (info.is_online ? "online" : "offline"));
        });
        
        // Job status callback
        job_queue.set_job_status_callback([](const AllPress::PrintJob& job) {
            LOG_INFO("Job " + std::to_string(job.job_id) + " status: " + 
                    std::to_string(static_cast<int>(job.status)));
        });
        
        // Start REST API server
        LOG_INFO("Starting REST API server...");
        AllPress::API::RESTServer rest_server(port, &printer_manager, &job_queue);
        rest_server.start();
        
        // Start WebSocket server
        LOG_INFO("Starting WebSocket server...");
        AllPress::API::init_websocket_server(ws_port);
        
        LOG_INFO("===========================================");
        LOG_INFO("All Press C++ Server is running");
        LOG_INFO("HTTP API: http://localhost:" + std::to_string(port));
        LOG_INFO("WebSocket: ws://localhost:" + std::to_string(ws_port));
        LOG_INFO("Press Ctrl+C to stop");
        LOG_INFO("===========================================");
        
        // Main server loop
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // Display statistics every 30 seconds
            static int counter = 0;
            if (++counter >= 30) {
                counter = 0;
                
                size_t queue_size = job_queue.get_queue_size();
                size_t active_jobs = job_queue.get_active_job_count();
                
                LOG_INFO("Stats - Queue: " + std::to_string(queue_size) + 
                        ", Active: " + std::to_string(active_jobs));
            }
        }
        
        // Shutdown
        LOG_INFO("Shutting down All Press C++ Server...");
        
        AllPress::API::stop_websocket_server();
        rest_server.stop();
        printer_manager.stop_status_monitoring();
        job_queue.stop();
        
        LOG_INFO("Server stopped successfully");
        
    } catch (const std::exception& e) {
        LOG_CRITICAL("Fatal error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}

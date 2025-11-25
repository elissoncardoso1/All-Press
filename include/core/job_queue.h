#pragma once

#include <queue>
#include <memory>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <optional>
#include "printer_manager.h"
#include "protocols/plotter_protocol_base.h"

namespace AllPress {

enum class JobStatus {
    Pending,
    Processing,
    Printing,
    Completed,
    Failed,
    Cancelled,
    Paused
};

struct PrintJob {
    int job_id;
    std::string printer_name;
    std::string file_path;
    std::string original_filename;
    PrintOptions options;
    JobStatus status = JobStatus::Pending;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point started_at;
    std::chrono::system_clock::time_point completed_at;
    int cups_job_id = 0;
    std::string error_message;
    float progress = 0.0f;
    size_t file_size = 0;
    int estimated_pages = 0;
    double estimated_cost = 0.0;
};

class JobQueue {
public:
    JobQueue(size_t max_concurrent_jobs = 4);
    ~JobQueue();
    
    // Gerenciamento de Jobs
    int add_job(const PrintJob& job);
    bool cancel_job(int job_id);
    bool pause_job(int job_id);
    bool resume_job(int job_id);
    bool retry_job(int job_id);  // 🆕 Tentar imprimir novamente
    bool move_job(int job_id, const std::string& new_printer);
    
    // Status e Consultas
    std::optional<PrintJob> get_job(int job_id);
    std::vector<PrintJob> get_jobs_for_printer(const std::string& printer);
    std::vector<PrintJob> get_active_jobs();
    std::vector<PrintJob> get_completed_jobs(int limit = 100);
    
    // Estatísticas
    size_t get_queue_size() const;
    size_t get_active_job_count() const;
    double get_estimated_queue_time(const std::string& printer);
    
    // Callbacks para eventos
    void set_job_status_callback(std::function<void(const PrintJob&)> callback);
    void set_progress_callback(std::function<void(int, float)> callback);
    
    void start();
    void stop();
    
    void set_printer_manager(PrinterManager* manager) { printer_manager_ = manager; }

private:
    // 🆕 Estrutura de contexto para processamento com protocolo
    struct ProcessingContext {
        int job_id;
        PrintJob job;
        std::unique_ptr<all_press::protocols::PlotterProtocolBase> protocol_handler;
        std::string target_protocol;
        all_press::protocols::PlotterCapabilities target_capabilities;
    };
    
    void worker_thread();
    void process_job(PrintJob& job);
    bool execute_print_job(PrintJob& job);
    void update_job_status(int job_id, JobStatus status, const std::string& error = "");
    
    // 🆕 Worker melhorado com conversão de protocolo
    void process_job_with_protocol(const ProcessingContext& context);
    
    // 🆕 Pre-flight checks
    bool validate_job_compatibility(const PrintJob& job);
    
    std::queue<std::shared_ptr<PrintJob>> job_queue_;
    std::unordered_map<int, std::shared_ptr<PrintJob>> jobs_map_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_{false};
    size_t max_concurrent_jobs_;
    std::atomic<size_t> active_jobs_{0};
    std::atomic<int> next_job_id_{1};
    
    std::function<void(const PrintJob&)> status_callback_;
    std::function<void(int, float)> progress_callback_;
    
    PrinterManager* printer_manager_;
    
    // 🆕 Cache de protocolos
    std::map<std::string, std::unique_ptr<all_press::protocols::PlotterProtocolBase>> 
        protocol_cache_;
    std::mutex protocol_cache_mutex_;
};

} // namespace AllPress

#include "core/job_queue.h"
#include "utils/logger.h"
#include "utils/file_utils.h"
#include <algorithm>

namespace AllPress {

JobQueue::JobQueue(size_t max_concurrent_jobs) 
    : max_concurrent_jobs_(max_concurrent_jobs), printer_manager_(nullptr) {
    LOG_INFO("JobQueue initialized with " + std::to_string(max_concurrent_jobs) + " workers");
}

JobQueue::~JobQueue() {
    stop();
}

int JobQueue::add_job(const PrintJob& job) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    PrintJob new_job = job;
    new_job.job_id = next_job_id_++;
    new_job.created_at = std::chrono::system_clock::now();
    new_job.status = JobStatus::Pending;
    
    auto job_ptr = std::make_shared<PrintJob>(new_job);
    jobs_map_[new_job.job_id] = job_ptr;
    job_queue_.push(job_ptr);
    
    queue_cv_.notify_one();
    
    LOG_INFO("Job added: " + std::to_string(new_job.job_id) + " for printer " + job.printer_name);
    return new_job.job_id;
}

bool JobQueue::cancel_job(int job_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    auto it = jobs_map_.find(job_id);
    if (it != jobs_map_.end()) {
        it->second->status = JobStatus::Cancelled;
        LOG_INFO("Job cancelled: " + std::to_string(job_id));
        
        if (status_callback_) {
            status_callback_(*it->second);
        }
        return true;
    }
    return false;
}

bool JobQueue::pause_job(int job_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    auto it = jobs_map_.find(job_id);
    if (it != jobs_map_.end()) {
        it->second->status = JobStatus::Paused;
        LOG_INFO("Job paused: " + std::to_string(job_id));
        return true;
    }
    return false;
}

bool JobQueue::resume_job(int job_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    auto it = jobs_map_.find(job_id);
    if (it != jobs_map_.end() && it->second->status == JobStatus::Paused) {
        it->second->status = JobStatus::Pending;
        LOG_INFO("Job resumed: " + std::to_string(job_id));
        return true;
    }
    return false;
}

bool JobQueue::retry_job(int job_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    auto it = jobs_map_.find(job_id);
    if (it != jobs_map_.end()) {
        // Só permite retry de jobs que falh aram ou foram cancelados
        if (it->second->status == JobStatus::Failed || 
            it->second->status == JobStatus::Cancelled) {
            
            // Resetar status e limpar mensagem de erro
            it->second->status = JobStatus::Pending;
            it->second->error_message.clear();
            it->second->progress = 0.0f;
            it->second->started_at = std::chrono::system_clock::time_point();
            it->second->completed_at = std::chrono::system_clock::time_point();
            
            // Adicionar novamente à fila
            job_queue_.push(it->second);
            queue_cv_.notify_one();
            
            LOG_INFO("Job " + std::to_string(job_id) + " queued for retry");
            
            if (status_callback_) {
                status_callback_(*it->second);
            }
            
            return true;
        } else {
            LOG_WARNING("Job " + std::to_string(job_id) + 
                       " cannot be retried (status: " + 
                       std::to_string(static_cast<int>(it->second->status)) + ")");
        }
    }
    return false;
}

bool JobQueue::move_job(int job_id, const std::string& new_printer) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    auto it = jobs_map_.find(job_id);
    if (it != jobs_map_.end()) {
        it->second->printer_name = new_printer;
        LOG_INFO("Job " + std::to_string(job_id) + " moved to printer " + new_printer);
        return true;
    }
    return false;
}

std::optional<PrintJob> JobQueue::get_job(int job_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    auto it = jobs_map_.find(job_id);
    if (it != jobs_map_.end()) {
        return *it->second;
    }
    return std::nullopt;
}

std::vector<PrintJob> JobQueue::get_jobs_for_printer(const std::string& printer) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    std::vector<PrintJob> result;
    for (const auto& pair : jobs_map_) {
        if (pair.second->printer_name == printer) {
            result.push_back(*pair.second);
        }
    }
    return result;
}

std::vector<PrintJob> JobQueue::get_active_jobs() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    std::vector<PrintJob> result;
    for (const auto& pair : jobs_map_) {
        if (pair.second->status == JobStatus::Processing || 
            pair.second->status == JobStatus::Printing) {
            result.push_back(*pair.second);
        }
    }
    return result;
}

std::vector<PrintJob> JobQueue::get_completed_jobs(int limit) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    std::vector<PrintJob> result;
    for (const auto& pair : jobs_map_) {
        if (pair.second->status == JobStatus::Completed || 
            pair.second->status == JobStatus::Failed) {
            result.push_back(*pair.second);
            if (result.size() >= static_cast<size_t>(limit)) break;
        }
    }
    return result;
}

size_t JobQueue::get_queue_size() const {
    return job_queue_.size();
}

size_t JobQueue::get_active_job_count() const {
    return active_jobs_.load();
}

double JobQueue::get_estimated_queue_time(const std::string& printer) {
    // Simple estimation: 30 seconds per job
    auto jobs = get_jobs_for_printer(printer);
    return jobs.size() * 30.0;
}

void JobQueue::set_job_status_callback(std::function<void(const PrintJob&)> callback) {
    status_callback_ = callback;
}

void JobQueue::set_progress_callback(std::function<void(int, float)> callback) {
    progress_callback_ = callback;
}

void JobQueue::start() {
    running_ = true;
    
    for (size_t i = 0; i < max_concurrent_jobs_; ++i) {
        worker_threads_.emplace_back(&JobQueue::worker_thread, this);
    }
    
    LOG_INFO("JobQueue started with " + std::to_string(max_concurrent_jobs_) + " workers");
}

void JobQueue::stop() {
    running_ = false;
    queue_cv_.notify_all();
    
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
    LOG_INFO("JobQueue stopped");
}

void JobQueue::worker_thread() {
    while (running_) {
        std::shared_ptr<PrintJob> job;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { 
                return !running_ || !job_queue_.empty(); 
            });
            
            if (!running_) break;
            
            if (!job_queue_.empty()) {
                job = job_queue_.front();
                job_queue_.pop();
            }
        }
        
        if (job) {
            process_job(*job);
        }
    }
}

void JobQueue::process_job(PrintJob& job) {
    if (job.status == JobStatus::Cancelled || job.status == JobStatus::Paused) {
        return;
    }
    
    active_jobs_++;
    job.status = JobStatus::Processing;
    job.started_at = std::chrono::system_clock::now();
    
    LOG_INFO("Processing job " + std::to_string(job.job_id));
    
    if (status_callback_) {
        status_callback_(job);
    }
    
    bool success = execute_print_job(job);
    
    if (success) {
        job.status = JobStatus::Completed;
        job.completed_at = std::chrono::system_clock::now();
        LOG_INFO("Job completed: " + std::to_string(job.job_id));
    } else {
        job.status = JobStatus::Failed;
        job.error_message = "Print job failed";
        LOG_ERROR("Job failed: " + std::to_string(job.job_id));
    }
    
    if (status_callback_) {
        status_callback_(job);
    }
    
    active_jobs_--;
}

bool JobQueue::execute_print_job(PrintJob& job) {
    if (!printer_manager_) {
        LOG_ERROR("PrinterManager not set");
        job.error_message = "Printer manager not available";
        return false;
    }
    
    // Check if file exists
    if (!Utils::FileUtils::file_exists(job.file_path)) {
        LOG_ERROR("File does not exist: " + job.file_path);
        job.error_message = "File not found: " + job.file_path;
        return false;
    }
    
    // Simulate progress
    for (int i = 0; i <= 100; i += 20) {
        if (job.status == JobStatus::Cancelled) {
            return false;
        }
        
        job.progress = i / 100.0f;
        if (progress_callback_) {
            progress_callback_(job.job_id, job.progress);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    job.status = JobStatus::Printing;
    LOG_INFO("Submitting print job to printer: " + job.printer_name + " with file: " + job.file_path);
    
    int cups_job_id = printer_manager_->submit_print_job(
        job.printer_name, job.file_path, job.options);
    
    if (cups_job_id > 0) {
        job.cups_job_id = cups_job_id;
        LOG_INFO("Print job submitted successfully with CUPS job ID: " + std::to_string(cups_job_id));
        return true;
    } else {
        LOG_ERROR("Failed to submit print job to printer: " + job.printer_name);
        job.error_message = "Failed to submit print job. Check printer connection and file format.";
        return false;
    }
}

void JobQueue::update_job_status(int job_id, JobStatus status, const std::string& error) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    auto it = jobs_map_.find(job_id);
    if (it != jobs_map_.end()) {
        it->second->status = status;
        if (!error.empty()) {
            it->second->error_message = error;
        }
        
        if (status_callback_) {
            status_callback_(*it->second);
        }
    }
}

} // namespace AllPress

#include "utils/logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>

namespace AllPress {
namespace Utils {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : current_level_(LogLevel::INFO), console_output_(true) {
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::set_level(LogLevel level) {
    current_level_ = level;
}

void Logger::set_output_file(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    log_file_.open(file_path, std::ios::app);
}

void Logger::enable_console_output(bool enable) {
    console_output_ = enable;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string log_message = "[" + get_timestamp() + "] " +
                             "[" + level_to_string(level) + "] " +
                             message;
    
    if (console_output_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << log_message << std::endl;
        } else {
            std::cout << log_message << std::endl;
        }
    }
    
    if (log_file_.is_open()) {
        log_file_ << log_message << std::endl;
        log_file_.flush();
    }
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

std::string Logger::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace Utils
} // namespace AllPress

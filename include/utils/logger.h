#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <fstream>
#include <sstream>

namespace AllPress::Utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static Logger& instance();
    
    void set_level(LogLevel level);
    void set_output_file(const std::string& file_path);
    void enable_console_output(bool enable);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    template<typename... Args>
    void debug(const std::string& format, Args... args) {
        log(LogLevel::DEBUG, format_message(format, args...));
    }
    
    template<typename... Args>
    void info(const std::string& format, Args... args) {
        log(LogLevel::INFO, format_message(format, args...));
    }
    
    template<typename... Args>
    void error(const std::string& format, Args... args) {
        log(LogLevel::ERROR, format_message(format, args...));
    }

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void log(LogLevel level, const std::string& message);
    std::string level_to_string(LogLevel level);
    std::string get_timestamp();
    
    template<typename... Args>
    std::string format_message(const std::string& format, Args... args) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), format.c_str(), args...);
        return std::string(buffer);
    }
    
    LogLevel current_level_;
    std::ofstream log_file_;
    bool console_output_;
    std::mutex log_mutex_;
};

// Convenience macros
#define LOG_DEBUG(msg) AllPress::Utils::Logger::instance().debug(msg)
#define LOG_INFO(msg) AllPress::Utils::Logger::instance().info(msg)
#define LOG_WARNING(msg) AllPress::Utils::Logger::instance().warning(msg)
#define LOG_ERROR(msg) AllPress::Utils::Logger::instance().error(msg)
#define LOG_CRITICAL(msg) AllPress::Utils::Logger::instance().critical(msg)

} // namespace AllPress::Utils

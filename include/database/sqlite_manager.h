#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <mutex>
#include <chrono>

namespace AllPress::Database {

struct Printer {
    int id;
    std::string name;
    std::string uri;
    std::string description;
    std::string location;
    std::string make_model;
    std::string status;
    std::string capabilities; // JSON
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

struct Job {
    int id;
    int printer_id;
    std::string file_path;
    std::string original_filename;
    std::string status;
    int pages;
    int copies;
    bool color;
    bool duplex;
    std::string paper_size;
    double cost;
    std::string client_name;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point completed_at;
};

class SQLiteManager {
public:
    SQLiteManager(const std::string& db_path = "all_press.db");
    ~SQLiteManager();
    
    bool initialize();
    bool migrate();
    
    // Printer operations
    int insert_printer(const Printer& printer);
    bool update_printer(const Printer& printer);
    bool delete_printer(int printer_id);
    std::optional<Printer> get_printer(int printer_id);
    std::optional<Printer> get_printer_by_name(const std::string& name);
    std::vector<Printer> get_all_printers();
    
    // Job operations
    int insert_job(const Job& job);
    bool update_job(const Job& job);
    bool delete_job(int job_id);
    std::optional<Job> get_job(int job_id);
    std::vector<Job> get_jobs_for_printer(int printer_id);
    std::vector<Job> get_jobs_by_status(const std::string& status);
    std::vector<Job> get_recent_jobs(int limit = 100);
    
    // Statistics
    int get_total_pages_printed(const std::string& date_range = "");
    double get_total_revenue(const std::string& date_range = "");
    
    // Cleanup
    bool cleanup_old_jobs(int days_old = 30);
    
private:
    bool execute_sql(const std::string& sql);
    sqlite3_stmt* prepare_statement(const std::string& sql);
    
    sqlite3* db_;
    std::string db_path_;
    std::mutex db_mutex_;
    
    static const char* CREATE_TABLES_SQL;
};

} // namespace AllPress::Database

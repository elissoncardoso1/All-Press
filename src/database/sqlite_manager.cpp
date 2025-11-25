#include "database/sqlite_manager.h"
#include "utils/logger.h"
#include <ctime>
#include <sstream>

namespace AllPress {
namespace Database {

const char* SQLiteManager::CREATE_TABLES_SQL = R"(
CREATE TABLE IF NOT EXISTS printers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    uri TEXT NOT NULL,
    description TEXT,
    location TEXT,
    make_model TEXT,
    status TEXT DEFAULT 'idle',
    capabilities TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS jobs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    printer_id INTEGER NOT NULL,
    file_path TEXT NOT NULL,
    original_filename TEXT NOT NULL,
    status TEXT DEFAULT 'pending',
    pages INTEGER DEFAULT 0,
    copies INTEGER DEFAULT 1,
    color BOOLEAN DEFAULT 1,
    duplex BOOLEAN DEFAULT 0,
    paper_size TEXT DEFAULT 'A4',
    cost REAL DEFAULT 0.0,
    client_name TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP,
    FOREIGN KEY (printer_id) REFERENCES printers(id)
);

CREATE INDEX IF NOT EXISTS idx_jobs_status ON jobs(status);
CREATE INDEX IF NOT EXISTS idx_jobs_printer ON jobs(printer_id);
CREATE INDEX IF NOT EXISTS idx_jobs_created ON jobs(created_at);
)";

SQLiteManager::SQLiteManager(const std::string& db_path) 
    : db_(nullptr), db_path_(db_path) {
}

SQLiteManager::~SQLiteManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool SQLiteManager::initialize() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    LOG_INFO("Database opened: " + db_path_);
    
    if (!execute_sql(CREATE_TABLES_SQL)) {
        LOG_ERROR("Failed to create tables");
        return false;
    }
    
    return true;
}

bool SQLiteManager::migrate() {
    LOG_INFO("Running database migrations");
    // Add migration logic here
    return true;
}

int SQLiteManager::insert_printer(const Printer& printer) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "INSERT INTO printers (name, uri, description, location, make_model, status, capabilities) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return -1;
    
    sqlite3_bind_text(stmt, 1, printer.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, printer.uri.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, printer.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, printer.location.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, printer.make_model.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, printer.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, printer.capabilities.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        return sqlite3_last_insert_rowid(db_);
    }
    
    return -1;
}

bool SQLiteManager::update_printer(const Printer& printer) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "UPDATE printers SET uri=?, description=?, location=?, make_model=?, "
                     "status=?, capabilities=?, updated_at=CURRENT_TIMESTAMP WHERE id=?";
    
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return false;
    
    sqlite3_bind_text(stmt, 1, printer.uri.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, printer.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, printer.location.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, printer.make_model.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, printer.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, printer.capabilities.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, printer.id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE);
}

bool SQLiteManager::delete_printer(int printer_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "DELETE FROM printers WHERE id=?";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return false;
    
    sqlite3_bind_int(stmt, 1, printer_id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE);
}

std::optional<Printer> SQLiteManager::get_printer(int printer_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "SELECT * FROM printers WHERE id=?";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return std::nullopt;
    
    sqlite3_bind_int(stmt, 1, printer_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Printer printer;
        printer.id = sqlite3_column_int(stmt, 0);
        printer.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        printer.uri = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        sqlite3_finalize(stmt);
        return printer;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<Printer> SQLiteManager::get_printer_by_name(const std::string& name) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "SELECT * FROM printers WHERE name=?";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return std::nullopt;
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Printer printer;
        printer.id = sqlite3_column_int(stmt, 0);
        printer.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        printer.uri = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        sqlite3_finalize(stmt);
        return printer;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Printer> SQLiteManager::get_all_printers() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<Printer> printers;
    
    const char* sql = "SELECT * FROM printers ORDER BY name";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return printers;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Printer printer;
        printer.id = sqlite3_column_int(stmt, 0);
        printer.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        printer.uri = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        printers.push_back(printer);
    }
    
    sqlite3_finalize(stmt);
    return printers;
}

int SQLiteManager::insert_job(const Job& job) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "INSERT INTO jobs (printer_id, file_path, original_filename, status, pages, copies, "
                     "color, duplex, paper_size, cost, client_name) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, job.printer_id);
    sqlite3_bind_text(stmt, 2, job.file_path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, job.original_filename.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, job.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, job.pages);
    sqlite3_bind_int(stmt, 6, job.copies);
    sqlite3_bind_int(stmt, 7, job.color ? 1 : 0);
    sqlite3_bind_int(stmt, 8, job.duplex ? 1 : 0);
    sqlite3_bind_text(stmt, 9, job.paper_size.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 10, job.cost);
    sqlite3_bind_text(stmt, 11, job.client_name.c_str(), -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        return sqlite3_last_insert_rowid(db_);
    }
    
    return -1;
}

bool SQLiteManager::update_job(const Job& job) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "UPDATE jobs SET status=?, pages=?, cost=?, completed_at=? WHERE id=?";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return false;
    
    sqlite3_bind_text(stmt, 1, job.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, job.pages);
    sqlite3_bind_double(stmt, 3, job.cost);
    // sqlite3_bind_text for timestamp
    sqlite3_bind_int(stmt, 5, job.id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE);
}

bool SQLiteManager::delete_job(int job_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "DELETE FROM jobs WHERE id=?";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return false;
    
    sqlite3_bind_int(stmt, 1, job_id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE);
}

std::optional<Job> SQLiteManager::get_job(int job_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "SELECT * FROM jobs WHERE id=?";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return std::nullopt;
    
    sqlite3_bind_int(stmt, 1, job_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Job job;
        job.id = sqlite3_column_int(stmt, 0);
        job.printer_id = sqlite3_column_int(stmt, 1);
        job.file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        job.original_filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        job.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        
        sqlite3_finalize(stmt);
        return job;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Job> SQLiteManager::get_jobs_for_printer(int printer_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<Job> jobs;
    
    const char* sql = "SELECT * FROM jobs WHERE printer_id=? ORDER BY created_at DESC";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return jobs;
    
    sqlite3_bind_int(stmt, 1, printer_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Job job;
        job.id = sqlite3_column_int(stmt, 0);
        job.printer_id = sqlite3_column_int(stmt, 1);
        job.file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        jobs.push_back(job);
    }
    
    sqlite3_finalize(stmt);
    return jobs;
}

std::vector<Job> SQLiteManager::get_jobs_by_status(const std::string& status) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<Job> jobs;
    
    const char* sql = "SELECT * FROM jobs WHERE status=? ORDER BY created_at DESC";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return jobs;
    
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Job job;
        job.id = sqlite3_column_int(stmt, 0);
        job.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        jobs.push_back(job);
    }
    
    sqlite3_finalize(stmt);
    return jobs;
}

std::vector<Job> SQLiteManager::get_recent_jobs(int limit) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<Job> jobs;
    
    std::string sql = "SELECT * FROM jobs ORDER BY created_at DESC LIMIT " + std::to_string(limit);
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return jobs;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Job job;
        job.id = sqlite3_column_int(stmt, 0);
        jobs.push_back(job);
    }
    
    sqlite3_finalize(stmt);
    return jobs;
}

int SQLiteManager::get_total_pages_printed(const std::string& date_range) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "SELECT SUM(pages * copies) FROM jobs WHERE status='completed'";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return 0;
    
    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return total;
}

double SQLiteManager::get_total_revenue(const std::string& date_range) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    const char* sql = "SELECT SUM(cost) FROM jobs WHERE status='completed'";
    sqlite3_stmt* stmt = prepare_statement(sql);
    if (!stmt) return 0.0;
    
    double total = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_double(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return total;
}

bool SQLiteManager::cleanup_old_jobs(int days_old) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    std::string sql = "DELETE FROM jobs WHERE created_at < datetime('now', '-" + 
                     std::to_string(days_old) + " days') AND status IN ('completed', 'failed', 'cancelled')";
    
    return execute_sql(sql);
}

bool SQLiteManager::execute_sql(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        LOG_ERROR("SQL error: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

sqlite3_stmt* SQLiteManager::prepare_statement(const std::string& sql) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return nullptr;
    }
    
    return stmt;
}

} // namespace Database
} // namespace AllPress

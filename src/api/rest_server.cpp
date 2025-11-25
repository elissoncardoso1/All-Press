#include "api/rest_server.h"
#include "conversion/file_processor.h"
#include "core/job_queue.h"
#include "utils/file_utils.h"
#include "utils/logger.h"
#include "utils/config.h"
#include <crow.h>
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

namespace AllPress {
namespace API {

// Função auxiliar para criar PDF básico a partir de texto
std::string create_basic_pdf_from_text(const std::string &filename);

class RESTServer::Impl {
public:
  Impl(int port, PrinterManager *printer_mgr, JobQueue *job_queue)
      : port_(port), printer_mgr_(printer_mgr), job_queue_(job_queue) {
    setup_routes();
  }

  ~Impl() { stop(); }

  void start() {
    if (running_)
      return;

    running_ = true;
    server_thread_ = std::thread([this]() {
      LOG_INFO("REST Server starting on port " + std::to_string(port_));
      app_.port(port_).multithreaded().run();
    });
  }

  void stop() {
    if (!running_)
      return;

    running_ = false;
    app_.stop();
    if (server_thread_.joinable()) {
      server_thread_.join();
    }
    LOG_INFO("REST Server stopped");
  }

  bool is_running() const { return running_; }

private:
  crow::SimpleApp app_;
  int port_;
  PrinterManager *printer_mgr_;
  JobQueue *job_queue_;
  std::atomic<bool> running_{false};
  std::thread server_thread_;

  void setup_routes() {
    // Helper function to add CORS headers
    auto add_cors = [](crow::response& res) {
      res.add_header("Access-Control-Allow-Origin", "*");
      res.add_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS, PUT, PATCH");
      res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    };

    // GET /
    CROW_ROUTE(app_, "/")
    ([]() { return crow::response(200, "All Press API is running"); });

    // GET /api/printers
    CROW_ROUTE(app_, "/api/printers")
    ([this]() {
      if (!printer_mgr_)
        return crow::response(500);

      auto printers = printer_mgr_->get_all_printers();
      json j_printers = json::array();

      for (const auto &p : printers) {
        j_printers.push_back(
            {{"id", p.name},
             {"name", p.name},
             {"status", p.is_online ? "online" : "offline"},
             {"type", "CUPS"},
             {"manufacturer", p.make_model},
             {"model", p.make_model},
             {"location", p.location},
             {"uri", p.uri},
             {"capabilities",
              {{"supportedFormats",
                {"pdf", "jpg", "png", "docx", "doc", "xlsx", "xls", "pptx",
                 "ppt", "dwg", "dxf", "svg", "ai", "psd", "cdr", "eps"}},
               {"colorSupported", true},
               {"duplexSupported", true},
               {"maxPaperSize", "A4"},
               {"resolutions", {300, 600}},
               {"paperSizes", {"A4", "Letter"}}}},
             {"currentJobs", p.jobs_count},
             {"totalJobsProcessed", 0}});
      }
      auto res = crow::response(j_printers.dump());
      res.add_header("Access-Control-Allow-Origin", "*");
      return res;
    });

    // POST /api/printers/discover
    CROW_ROUTE(app_, "/api/printers/discover")
        .methods(crow::HTTPMethod::POST)([this]() {
          if (!printer_mgr_)
            return crow::response(500);
          printer_mgr_->discover_cups_printers();
          // Redirect to get printers logic or just call it
          // For simplicity, returning empty array as discovery is async or we
          // just re-fetch But the original code called handle_get_printers()
          return crow::response(307); // Temporary Redirect to /api/printers?
                                      // No, just return the list.
        });

    // GET /api/printers/<string>
    CROW_ROUTE(app_, "/api/printers/<string>")
    ([this](std::string id) {
      // Implementation for getting specific printer
      if (!printer_mgr_)
        return crow::response(500);
      auto printers = printer_mgr_->get_all_printers();
      for (const auto &p : printers) {
        if (p.name == id) {
          // Construct JSON for single printer (reuse logic if possible)
          json j = {
              {"id", p.name},
              {"name", p.name},
              {"status", p.is_online ? "online" : "offline"},
              {"type", "CUPS"},
              {"manufacturer", p.make_model},
              {"model", p.make_model},
              {"location", p.location},
              {"uri", p.uri},
              {"capabilities",
               {{"supportedFormats",
                 {"pdf", "jpg", "png", "docx", "doc", "xlsx", "xls", "pptx",
                  "ppt", "dwg", "dxf", "svg", "ai", "psd", "cdr", "eps"}},
                {"colorSupported", true},
                {"duplexSupported", true},
                {"maxPaperSize", "A4"},
                {"resolutions", {300, 600}},
                {"paperSizes", {"A4", "Letter"}}}},
              {"currentJobs", p.jobs_count},
              {"totalJobsProcessed", 0}};
          return crow::response(j.dump());
        }
      }
      return crow::response(404, "Printer not found");
    });

    // POST /api/jobs
    CROW_ROUTE(app_, "/api/jobs")
        .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
          if (!job_queue_ || !printer_mgr_)
            return crow::response(500);

          crow::multipart::message msg(req);

          std::string printer_id = "default";
          std::string options_json = "{}";
          std::string filename = "uploaded_file.pdf";
          std::string file_content;

          if (msg.parts.size() == 0) {
            return crow::response(400, "No multipart data");
          }

          for (auto &part : msg.parts) {
            const auto &part_name = part.headers.find("Content-Disposition");
            if (part_name != part.headers.end()) {
              auto content_disposition =
                  part.get_header_object("Content-Disposition");

              // Check for "name" parameter
              auto name_it = content_disposition.params.find("name");
              if (name_it != content_disposition.params.end()) {
                std::string name = name_it->second;

                if (name == "printer_id") {
                  printer_id = part.body;
                } else if (name == "options") {
                  options_json = part.body;
                } else if (name == "file") {
                  auto filename_it =
                      content_disposition.params.find("filename");
                  if (filename_it != content_disposition.params.end()) {
                    filename = filename_it->second;
                  }
                  file_content = part.body;
                }
              }
            }
          }

          // ... (Rest of the logic similar to original handle_post_jobs)
          // Create temp file
          std::string temp_file =
              Utils::FileUtils::create_temp_file("allpress_upload_", ".pdf");

          if (!file_content.empty()) {
            std::ofstream out_file(temp_file, std::ios::binary);
            if (out_file.is_open()) {
              out_file.write(file_content.c_str(), file_content.length());
              out_file.close();
            }
          } else {
            // Fallback PDF creation
            std::string basic_pdf = create_basic_pdf_from_text(filename);
            std::ofstream out_file(temp_file, std::ios::binary);
            out_file.write(basic_pdf.c_str(), basic_pdf.length());
            out_file.close();
          }

          // Create Job logic
          PrintJob new_job;
          new_job.printer_name =
              printer_id; // Logic to resolve default printer needed
          new_job.file_path = temp_file;
          new_job.original_filename = filename;
          new_job.options.media_size = "A4";
          new_job.file_size = Utils::FileUtils::get_file_size(temp_file);
          new_job.estimated_pages = 1;

          int job_id = job_queue_->add_job(new_job);

          if (job_id > 0) {
            json j = {{"id", std::to_string(job_id)},
                      {"printerId", printer_id},
                      {"printerName", printer_id},
                      {"fileName", filename},
                      {"status", "pending"},
                      {"progress", 0},
                      {"createdAt", std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch()).count()},
                      {"options", json::parse(options_json.empty() ? "{}" : options_json)},
                      {"message", "Job created successfully"}};
            
            auto response = crow::response(200, j.dump());
            response.add_header("Content-Type", "application/json");
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
          } else {
            json error_j = {{"error", "Failed to create job"}, {"success", false}};
            auto response = crow::response(500, error_j.dump());
            response.add_header("Content-Type", "application/json");
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
          }
        });

    // GET /api/jobs
    CROW_ROUTE(app_, "/api/jobs")
    ([this]() {
      if (!job_queue_)
        return crow::response(500);
      auto active_jobs = job_queue_->get_active_jobs();
      auto completed_jobs = job_queue_->get_completed_jobs(100);

      json j_jobs = json::array();
      // ... Populate JSON from jobs
      for (const auto &job : active_jobs) {
        j_jobs.push_back({{"id", job.job_id},
                          {"status", "processing"}, // Map enum to string
                          {"fileName", job.original_filename}});
      }
      for (const auto &job : completed_jobs) {
        j_jobs.push_back({{"id", job.job_id},
                          {"status", "completed"}, // Map enum to string
                          {"fileName", job.original_filename}});
      }
      return crow::response(j_jobs.dump());
    });

    // DELETE /api/printers/<string>
    CROW_ROUTE(app_, "/api/printers/<string>")
        .methods(crow::HTTPMethod::DELETE)([this](std::string id) {
          // Mock implementation - in real app would call
          // printer_mgr_->delete_printer(id) For now just return success
          return crow::response(200);
        });

    // POST /api/printers/<string>/pause
    CROW_ROUTE(app_, "/api/printers/<string>/pause")
        .methods(crow::HTTPMethod::POST)([this](std::string id) {
          // Mock implementation
          return crow::response(200);
        });

    // POST /api/printers/<string>/resume
    CROW_ROUTE(app_, "/api/printers/<string>/resume")
        .methods(crow::HTTPMethod::POST)([this](std::string id) {
          // Mock implementation
          return crow::response(200);
        });

    // GET /api/printers/<string>/jobs
    CROW_ROUTE(app_, "/api/printers/<string>/jobs")
    ([this](std::string id) {
      if (!job_queue_)
        return crow::response(500);
      auto jobs = job_queue_->get_jobs_for_printer(
          id); // Assuming this method exists or similar
      json j_jobs = json::array();
      // Populate
      return crow::response(j_jobs.dump());
    });

    // GET /api/jobs/history
    CROW_ROUTE(app_, "/api/jobs/history")
    ([this](const crow::request &req) {
      // Get limit param
      int limit = 100;
      if (req.url_params.get("limit")) {
        limit = std::stoi(req.url_params.get("limit"));
      }

      if (!job_queue_)
        return crow::response(500);
      auto jobs = job_queue_->get_completed_jobs(limit);

      json j_jobs = json::array();
      for (const auto &job : jobs) {
        j_jobs.push_back(
            {{"id", job.job_id},
             {"status", "completed"},
             {"fileName", job.original_filename},
             {"createdAt", std::chrono::duration_cast<std::chrono::seconds>(
                               job.created_at.time_since_epoch())
                               .count()}});
      }
      return crow::response(j_jobs.dump());
    });

    // POST /api/jobs/cancel-multiple
    CROW_ROUTE(app_, "/api/jobs/cancel-multiple")
        .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
          auto x = json::parse(req.body);
          std::vector<int> ids = x["job_ids"].get<std::vector<int>>();
          for (int id : ids) {
            if (job_queue_)
              job_queue_->cancel_job(id);
          }
          return crow::response(200);
        });

    // GET /api/jobs/<int>
    CROW_ROUTE(app_, "/api/jobs/<int>")
    ([this](int id) {
      if (!job_queue_)
        return crow::response(500);
      auto job_opt = job_queue_->get_job(id);
      if (job_opt.has_value()) {
        auto &job = job_opt.value();
        json j = {{"id", job.job_id},
                  {"status", "processing"}, // Simplify
                  {"fileName", job.original_filename}};
        return crow::response(j.dump());
      }
      return crow::response(404);
    });

    // POST /api/jobs/<int>/retry
    CROW_ROUTE(app_, "/api/jobs/<int>/retry")
        .methods(crow::HTTPMethod::POST)([this](int id) {
          if (!job_queue_)
            return crow::response(500);
          if (job_queue_->retry_job(id))
            return crow::response(200);
          return crow::response(400);
        });

    // POST /api/jobs/<int>/cancel
    CROW_ROUTE(app_, "/api/jobs/<int>/cancel")
        .methods(crow::HTTPMethod::POST)([this](int id) {
          if (!job_queue_)
            return crow::response(500);
          if (job_queue_->cancel_job(id))
            return crow::response(200);
          return crow::response(400);
        });

    // POST /api/files/upload
    CROW_ROUTE(app_, "/api/files/upload")
        .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
          crow::multipart::message msg(req);
          if (msg.parts.size() == 0)
            return crow::response(400);

          // Handle file upload similar to jobs but just return ID/URL
          std::string temp_file =
              Utils::FileUtils::create_temp_file("upload_", ".pdf");
          // ... save file logic ...

          json j = {{"id", "temp_id"}, {"url", "/api/files/temp_id/preview"}};
          return crow::response(j.dump());
        });

    // GET /api/files/<string>/preview
    CROW_ROUTE(app_, "/api/files/<string>/preview")
    ([this](std::string id) {
      // Return file content or redirect
      // For now mock
      json j = {{"url", "http://localhost:8000/mock.pdf"}};
      return crow::response(j.dump());
    });

    // GET /api/files/<string>/metadata
    CROW_ROUTE(app_, "/api/files/<string>/metadata")
    ([this](std::string id) {
      json j = {{"pages", 1}, {"size", 1024}};
      return crow::response(j.dump());
    });

    // GET /api/system/metrics
    CROW_ROUTE(app_, "/api/system/metrics")
    ([]() {
      json j = {{"cpuUsage", 10},
                {"memoryUsage", 20},
                {"diskUsage", 30},
                {"temperature", 40}};
      return crow::response(j.dump());
    });

    // GET /api/system/logs
    CROW_ROUTE(app_, "/api/system/logs")
    ([]() {
      json j = json::array();
      j.push_back({{"timestamp", "2023-01-01T00:00:00Z"},
                   {"level", "INFO"},
                   {"message", "System started"}});
      return crow::response(j.dump());
    });

    // GET /api/system/stats
    CROW_ROUTE(app_, "/api/system/stats")
    ([this]() {
      if (!printer_mgr_)
        return crow::response(500);

      auto printers = printer_mgr_->get_all_printers();
      int online_count = 0;
      for (const auto &p : printers) {
        if (p.is_online)
          online_count++;
      }

      json j = {{"printersOnline", online_count},
                {"printersTotal", printers.size()},
                {"jobsPending", 0},
                {"jobsProcessing", 0},
                {"jobsCompleted", 0},
                {"jobsFailed", 0},
                {"pagesTotal", 0},
                {"pagesToday", 0},
                {"estimatedCost", 0}};

      return crow::response(j.dump());
    });

    // GET /api/system/status
    CROW_ROUTE(app_, "/api/system/status")
    ([]() {
      json j = {{"status", "operational"},
                {"uptime", 12345}, // Mock uptime
                {"version", "1.1.0"}};
      return crow::response(j.dump());
    });

    // GET /api/system/settings
    CROW_ROUTE(app_, "/api/system/settings")
    ([]() {
      try {
        AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
        std::string settings_json = config.get_all_settings_json();
        return crow::response(200, settings_json);
      } catch (const std::exception& e) {
        json j = {{"error", "Failed to get settings: " + std::string(e.what())},
                  {"success", false}};
        return crow::response(500, j.dump());
      }
    });

    // POST /api/system/settings
    CROW_ROUTE(app_, "/api/system/settings")
        .methods("POST"_method)
    ([this](const crow::request& req) {
      try {
        json body = json::parse(req.body);
        AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
        
        // Update settings from JSON
        for (auto it = body.begin(); it != body.end(); ++it) {
          if (it.value().is_object()) {
            // Handle nested objects (sections)
            for (auto section_it = it.value().begin(); section_it != it.value().end(); ++section_it) {
              std::string key = std::string(it.key()) + "." + std::string(section_it.key());
              
              if (section_it.value().is_string()) {
                config.set_string(key, section_it.value().get<std::string>());
              } else if (section_it.value().is_number_integer()) {
                config.set_int(key, section_it.value().get<int>());
              } else if (section_it.value().is_boolean()) {
                config.set_bool(key, section_it.value().get<bool>());
              } else if (section_it.value().is_number_float()) {
                config.set_double(key, section_it.value().get<double>());
              }
            }
          } else {
            // Handle top-level keys
            std::string key = it.key();
            
            if (it.value().is_string()) {
              config.set_string(key, it.value().get<std::string>());
            } else if (it.value().is_number_integer()) {
              config.set_int(key, it.value().get<int>());
            } else if (it.value().is_boolean()) {
              config.set_bool(key, it.value().get<bool>());
            } else if (it.value().is_number_float()) {
              config.set_double(key, it.value().get<double>());
            }
          }
        }
        
        // Save to file - use default path if not set
        std::string config_path = "config/all_press.conf";
        if (!config.save_to_file(config_path)) {
          json j = {{"error", "Failed to save configuration file: " + config_path},
                    {"success", false}};
          return crow::response(500, j.dump());
        }
        
        json j = {{"success", true},
                  {"message", "Settings saved successfully"}};
        return crow::response(200, j.dump());
      } catch (const json::parse_error& e) {
        json j = {{"error", "Invalid JSON: " + std::string(e.what())},
                  {"success", false}};
        return crow::response(400, j.dump());
      } catch (const std::exception& e) {
        json j = {{"error", "Failed to save settings: " + std::string(e.what())},
                  {"success", false}};
        return crow::response(500, j.dump());
      }
    });
  }
};

// Implement create_basic_pdf_from_text (keep original implementation)
std::string create_basic_pdf_from_text(const std::string &filename) {
  return "%PDF-1.4\n"
         "1 0 obj\n"
         "<< /Type /Catalog /Pages 2 0 R >>\n"
         "endobj\n"
         "2 0 obj\n"
         "<< /Type /Pages /Kids [3 0 R] /Count 1 >>\n"
         "endobj\n"
         "3 0 obj\n"
         "<< /Type /Page /Parent 2 0 R /Resources 4 0 R /MediaBox [0 0 612 "
         "792] /Contents 5 0 R >>\n"
         "endobj\n"
         "4 0 obj\n"
         "<< /Font << /F1 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica "
         ">> >> >>\n"
         "endobj\n"
         "5 0 obj\n"
         "<< /Length 44 >>\n"
         "stream\n"
         "BT\n"
         "/F1 24 Tf\n"
         "100 700 Td\n"
         "(Test Print Job) Tj\n"
         "ET\n"
         "endstream\n"
         "endobj\n"
         "xref\n"
         "0 6\n"
         "0000000000 65535 f\n"
         "0000000009 00000 n\n"
         "0000000058 00000 n\n"
         "0000000115 00000 n\n"
         "0000000214 00000 n\n"
         "0000000304 00000 n\n"
         "trailer\n"
         "<< /Size 6 /Root 1 0 R >>\n"
         "startxref\n"
         "398\n"
         "%%EOF\n";
}

RESTServer::RESTServer(int port, PrinterManager *printer_mgr,
                       JobQueue *job_queue)
    : pImpl(std::make_unique<Impl>(port, printer_mgr, job_queue)) {}

RESTServer::~RESTServer() = default;

void RESTServer::start() { pImpl->start(); }
void RESTServer::stop() { pImpl->stop(); }
bool RESTServer::is_running() const { return pImpl->is_running(); }

} // namespace API
} // namespace AllPress

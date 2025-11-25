# All Press C++ - Sistema de Impressão de Alto Desempenho

## Visão Geral

**Nome da Aplicação:** All Press C++

**Descrição:** Sistema completo de gerenciamento de impressão de alta performance desenvolvido em C/C++ com APIs nativas e frontend React. Projetado para gráficas rápidas que necessitam de máximo desempenho, robustez e controle granular sobre operações de impressão.

**Stack Tecnológica:**
- **Core Engine:** C++17, CMake, Ninja/Make
- **APIs Nativas:** CUPS C API, Windows Print Spooler API, IPP Protocol
- **Network/Threading:** Boost.Asio, std::thread, OpenSSL
- **Web Framework:** C++ httplib ou Crow Framework
- **Database:** SQLite C API, PostgreSQL C API
- **Frontend:** React, TypeScript, Vite (mantido como interface)
- **Bindings:** Pybind11 para interoperabilidade Python (opcional)
- **Build System:** CMake com Conan/vcpkg para dependências

**Funcionalidades Principais:**
1. Motor de impressão nativo C++ com performance 10-50x superior
2. Descoberta assíncrona de impressoras com threading paralelo
3. Processamento de arquivos com streaming para documentos grandes
4. Sistema de filas inteligente com balanceamento de carga
5. Color management nativo com perfis ICC
6. API REST C++ de alta performance
7. WebSocket server nativo para tempo real

---

## 1. Arquitetura do Sistema

### 1.1 Componentes Core (C++)

**Estrutura de Diretórios:**
```
/all_press_cpp
  /src
    /core                    # Engine principal
      printer_manager.cpp
      job_queue.cpp
      file_processor.cpp
      color_manager.cpp
    /network                 # Descoberta e comunicação
      cups_client.cpp
      ipp_client.cpp
      windows_print.cpp
      network_scanner.cpp
    /conversion              # Processamento de arquivos
      pdf_processor.cpp
      image_processor.cpp
      office_converter.cpp
      cad_converter.cpp
    /api                     # REST API Server
      http_server.cpp
      websocket_server.cpp
      endpoints.cpp
    /database               # Persistência
      sqlite_manager.cpp
      models.cpp
    /utils                  # Utilitários
      logger.cpp
      config.cpp
      file_utils.cpp
  /include                  # Headers públicos
  /external                 # Dependências externas
  /python_bindings         # Bindings Python (opcional)
  /frontend                # Interface React (mantida)
  /tests                   # Testes C++
  CMakeLists.txt
  conanfile.txt
```

### 1.2 Dependências C++

**Core Libraries:**
- **libcups-dev** - API CUPS nativa
- **boost** - Asio, System, Thread, Filesystem
- **openssl** - Comunicação segura
- **sqlite3** - Banco de dados embarcado
- **nlohmann/json** - Processamento JSON
- **spdlog** - Logging de alta performance

**Processamento de Arquivo:**
- **poppler-cpp** - Processamento PDF nativo
- **ImageMagick++** - Manipulação de imagens
- **libheif** - Suporte HEIC/HEIF
- **libarchive** - Arquivos compactados
- **libcairo** - Renderização SVG

**Network/Web:**
- **httplib** - HTTP server leve
- **websocketpp** - WebSocket server
- **curl** - Cliente HTTP
- **mdns** - Descoberta mDNS/Bonjour

**Build Tools:**
- **CMake 3.20+**
- **Conan 2.0** ou **vcpkg** para gestão de pacotes
- **Ninja** ou **Make** para build
- **GCC 9+** ou **Clang 12+**

---

## 2. Core Engine (C++)

### 2.1 Printer Manager
```cpp
// include/core/printer_manager.h
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <future>
#include <cups/cups.h>

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
    std::vector<PrinterInfo> discover_windows_printers();
    
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

private:
    void update_printer_status();
    void monitor_thread_func();
    
    std::vector<PrinterInfo> printers_;
    std::mutex printers_mutex_;
    std::thread monitor_thread_;
    std::atomic<bool> monitoring_active_{false};
    std::vector<std::function<void(const PrinterInfo&)>> status_callbacks_;
    
    cups_dest_t* cups_dests_ = nullptr;
    int num_cups_dests_ = 0;
};

} // namespace AllPress
```

### 2.2 Job Queue System
```cpp
// include/core/job_queue.h
#pragma once

#include <queue>
#include <memory>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>

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

private:
    void worker_thread();
    void process_job(PrintJob& job);
    bool execute_print_job(PrintJob& job);
    void update_job_status(int job_id, JobStatus status, const std::string& error = "");
    
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
};

} // namespace AllPress
```

### 2.3 File Processor
```cpp
// include/conversion/file_processor.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>

namespace AllPress {

enum class FileType {
    Unknown,
    PDF,
    Image,
    Office,
    CAD,
    Archive,
    SVG
};

struct FileInfo {
    std::string file_path;
    std::string mime_type;
    FileType type;
    size_t size_bytes;
    int estimated_pages;
    std::string dimensions; // "width x height"
    int dpi;
    bool has_color;
};

struct ConversionOptions {
    int target_dpi = 300;
    bool compress = true;
    std::string color_profile;
    bool preserve_transparency = false;
    std::string output_format = "PDF";
    int max_width = 0;  // 0 = no limit
    int max_height = 0;
};

class FileProcessor {
public:
    FileProcessor();
    ~FileProcessor();
    
    // Análise de arquivo
    FileInfo analyze_file(const std::string& file_path);
    FileType detect_file_type(const std::string& file_path);
    std::string detect_mime_type(const std::string& file_path);
    
    // Conversão
    std::future<std::string> convert_to_pdf_async(const std::string& input_path,
                                                 const ConversionOptions& options = {});
    std::string convert_to_pdf(const std::string& input_path, 
                              const ConversionOptions& options = {});
    
    // Processadores específicos
    std::string convert_office_to_pdf(const std::string& input_path);
    std::string convert_image_to_pdf(const std::string& input_path, 
                                    const ConversionOptions& options);
    std::string convert_cad_to_pdf(const std::string& input_path);
    std::string convert_svg_to_pdf(const std::string& input_path);
    
    // Processamento de arquivos compactados
    std::vector<std::string> extract_archive(const std::string& archive_path,
                                             const std::string& extract_to);
    
    // Otimização
    std::string optimize_pdf_for_printing(const std::string& pdf_path,
                                          const ConversionOptions& options);
    
    // Preview
    std::string generate_preview_image(const std::string& file_path,
                                      int width = 200, int height = 200);
    std::vector<std::string> generate_page_previews(const std::string& pdf_path,
                                                   int max_pages = 10);

private:
    class ImageProcessor;
    class PDFProcessor; 
    class OfficeConverter;
    class CADConverter;
    
    std::unique_ptr<ImageProcessor> image_processor_;
    std::unique_ptr<PDFProcessor> pdf_processor_;
    std::unique_ptr<OfficeConverter> office_converter_;
    std::unique_ptr<CADConverter> cad_converter_;
    
    std::string temp_dir_;
    std::mutex conversion_mutex_;
};

} // namespace AllPress
```

---

## 3. Network Layer (C++)

### 3.1 IPP Client Nativo
```cpp
// include/network/ipp_client.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <cups/cups.h>

namespace AllPress::Network {

struct IPPPrinter {
    std::string name;
    std::string uri;
    std::string location;
    std::string make_model;
    std::vector<std::string> supported_formats;
    std::vector<std::string> supported_media;
    bool supports_color;
    bool supports_duplex;
    int max_width;
    int max_height;
};

class IPPClient {
public:
    IPPClient();
    ~IPPClient();
    
    // Descoberta assíncrona paralela
    std::future<std::vector<IPPPrinter>> discover_printers_async(
        const std::string& subnet = "192.168.1", 
        int timeout_ms = 5000);
    
    // Operações IPP
    bool print_document_stream(const std::string& printer_uri,
                              std::istream& document_stream,
                              const std::string& job_name,
                              const std::unordered_map<std::string, std::string>& attributes);
    
    IPPPrinter get_printer_attributes(const std::string& printer_uri);
    std::vector<int> get_active_jobs(const std::string& printer_uri);
    bool cancel_job(const std::string& printer_uri, int job_id);
    
    // Capacidades
    std::vector<std::string> get_supported_formats(const std::string& printer_uri);
    std::vector<std::string> get_supported_media(const std::string& printer_uri);
    
private:
    bool probe_ipp_printer(const std::string& ip, int port = 631);
    IPPPrinter query_printer_info(const std::string& uri);
    void scan_ip_range_parallel(const std::string& subnet, 
                               std::vector<IPPPrinter>& results,
                               std::mutex& results_mutex);
    
    http_t* http_connection_;
    std::string user_agent_;
};

} // namespace AllPress::Network
```

### 3.2 Network Scanner
```cpp
// include/network/network_scanner.h
#pragma once

#include <string>
#include <vector>
#include <thread>
#include <future>
#include <atomic>

namespace AllPress::Network {

struct NetworkDevice {
    std::string ip;
    std::vector<int> open_ports;
    std::string hostname;
    std::string mac_address;
    bool is_printer = false;
    std::string printer_type; // "IPP", "LPD", "Raw", "SMB"
};

class NetworkScanner {
public:
    NetworkScanner(size_t thread_count = std::thread::hardware_concurrency());
    ~NetworkScanner();
    
    // Scan de rede paralelo
    std::future<std::vector<NetworkDevice>> scan_subnet_async(
        const std::string& subnet,
        const std::vector<int>& ports = {631, 9100, 515, 80, 443, 139, 445});
    
    // mDNS/Bonjour discovery
    std::future<std::vector<NetworkDevice>> discover_mdns_devices_async(
        const std::vector<std::string>& service_types = {
            "_ipp._tcp", "_printer._tcp", "_pdl-datastream._tcp"
        });
    
    // Utilitários
    bool ping_host(const std::string& ip, int timeout_ms = 1000);
    bool check_port(const std::string& ip, int port, int timeout_ms = 1000);
    std::string resolve_hostname(const std::string& ip);
    
private:
    void scan_ip_worker(const std::string& base_ip, int start_host, int end_host,
                       const std::vector<int>& ports,
                       std::vector<NetworkDevice>& results,
                       std::mutex& results_mutex);
    
    bool identify_printer_service(const std::string& ip, int port);
    
    size_t thread_count_;
    std::vector<std::thread> worker_threads_;
};

} // namespace AllPress::Network
```

---

## 4. REST API Server (C++)

### 4.1 HTTP Server
```cpp
// include/api/http_server.h
#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <memory>

namespace AllPress::API {

using json = nlohmann::json;

class HTTPServer {
public:
    HTTPServer(int port = 8000);
    ~HTTPServer();
    
    void start();
    void stop();
    bool is_running() const;
    
private:
    void setup_routes();
    void setup_cors();
    void setup_middleware();
    
    // Printer endpoints
    void handle_get_printers(const httplib::Request& req, httplib::Response& res);
    void handle_get_printer(const httplib::Request& req, httplib::Response& res);
    void handle_discover_printers(const httplib::Request& req, httplib::Response& res);
    void handle_add_printer(const httplib::Request& req, httplib::Response& res);
    
    // Job endpoints  
    void handle_submit_job(const httplib::Request& req, httplib::Response& res);
    void handle_get_job(const httplib::Request& req, httplib::Response& res);
    void handle_cancel_job(const httplib::Request& req, httplib::Response& res);
    void handle_get_jobs(const httplib::Request& req, httplib::Response& res);
    
    // File endpoints
    void handle_upload_file(const httplib::Request& req, httplib::Response& res);
    void handle_convert_file(const httplib::Request& req, httplib::Response& res);
    void handle_preview_file(const httplib::Request& req, httplib::Response& res);
    
    // Pricing endpoints
    void handle_calculate_price(const httplib::Request& req, httplib::Response& res);
    void handle_get_pricing_rules(const httplib::Request& req, httplib::Response& res);
    
    // Utility methods
    json error_response(const std::string& message, int code = 400);
    json success_response(const json& data = json::object());
    void set_json_headers(httplib::Response& res);
    
    std::unique_ptr<httplib::Server> server_;
    std::thread server_thread_;
    std::atomic<bool> running_{false};
    int port_;
    
    // Service dependencies
    std::shared_ptr<PrinterManager> printer_manager_;
    std::shared_ptr<JobQueue> job_queue_;
    std::shared_ptr<FileProcessor> file_processor_;
};

} // namespace AllPress::API
```

### 4.2 WebSocket Server para Tempo Real
```cpp
// include/api/websocket_server.h
#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>

namespace AllPress::API {

typedef websocketpp::server<websocketpp::config::asio> WSServer;

struct WSConnection {
    websocketpp::connection_hdl hdl;
    std::string endpoint; // "/ws/printers", "/ws/jobs/{id}", etc
    std::string resource_id; // printer name, job id, etc
    std::chrono::system_clock::time_point connected_at;
};

class WebSocketServer {
public:
    WebSocketServer(int port = 8001);
    ~WebSocketServer();
    
    void start();
    void stop();
    
    // Broadcasting
    void broadcast_printer_status(const std::string& printer_name, const json& status);
    void broadcast_job_status(int job_id, const json& status);
    void broadcast_job_progress(int job_id, float progress);
    void broadcast_queue_status(const json& status);
    
private:
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, WSServer::message_ptr msg);
    
    void handle_subscribe(websocketpp::connection_hdl hdl, const json& message);
    void handle_unsubscribe(websocketpp::connection_hdl hdl, const json& message);
    
    void send_to_connection(websocketpp::connection_hdl hdl, const json& message);
    void broadcast_to_endpoint(const std::string& endpoint, const json& message);
    
    WSServer ws_server_;
    std::thread server_thread_;
    std::atomic<bool> running_{false};
    
    std::set<WSConnection> connections_;
    std::mutex connections_mutex_;
    int port_;
};

} // namespace AllPress::API
```

---

## 5. Database Layer (C++)

### 5.1 SQLite Manager
```cpp
// include/database/sqlite_manager.h
#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>

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
    std::vector<std::pair<std::string, int>> get_top_clients(int limit = 10);
    
    // Cleanup
    bool cleanup_old_jobs(int days_old = 30);
    
private:
    bool execute_sql(const std::string& sql);
    sqlite3_stmt* prepare_statement(const std::string& sql);
    void bind_printer(sqlite3_stmt* stmt, const Printer& printer);
    void bind_job(sqlite3_stmt* stmt, const Job& job);
    Printer extract_printer(sqlite3_stmt* stmt);
    Job extract_job(sqlite3_stmt* stmt);
    
    sqlite3* db_;
    std::string db_path_;
    std::mutex db_mutex_;
    
    static const char* CREATE_TABLES_SQL;
    static const char* MIGRATION_SQL[];
};

} // namespace AllPress::Database
```

---

## 6. Color Management (C++)

### 6.1 ICC Profile Manager
```cpp
// include/core/color_manager.h
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace AllPress::Color {

enum class RenderingIntent {
    Perceptual,
    RelativeColorimetric,
    Saturation,
    AbsoluteColorimetric
};

struct ColorProfile {
    std::string name;
    std::string file_path;
    std::string description;
    std::string device_class; // "input", "display", "output"
    std::string color_space; // "RGB", "CMYK", "Lab"
    bool is_default = false;
};

struct CalibrationData {
    std::string printer_name;
    std::chrono::system_clock::time_point calibrated_at;
    std::unordered_map<std::string, double> color_corrections;
    double gamma = 1.0;
    bool is_valid = true;
};

class ColorManager {
public:
    ColorManager();
    ~ColorManager();
    
    bool initialize();
    
    // Profile management
    bool load_profile(const std::string& file_path, const std::string& name = "");
    bool remove_profile(const std::string& name);
    std::vector<ColorProfile> get_available_profiles();
    ColorProfile get_profile(const std::string& name);
    
    // Color conversion
    bool convert_image_colors(const std::string& input_path,
                             const std::string& output_path,
                             const std::string& source_profile,
                             const std::string& target_profile,
                             RenderingIntent intent = RenderingIntent::Perceptual);
    
    // PDF color management
    bool apply_color_profile_to_pdf(const std::string& pdf_path,
                                   const std::string& output_path,
                                   const std::string& target_profile);
    
    // Printer calibration
    bool calibrate_printer(const std::string& printer_name,
                          const std::string& test_pattern_path = "");
    CalibrationData get_printer_calibration(const std::string& printer_name);
    bool apply_calibration(const std::string& printer_name,
                          const std::string& document_path);
    
    // Default profiles
    void set_default_input_profile(const std::string& profile_name);
    void set_default_output_profile(const std::string& printer_name,
                                   const std::string& profile_name);
    std::string get_default_profile_for_printer(const std::string& printer_name);
    
    // Built-in profiles
    bool install_standard_profiles(); // sRGB, Adobe RGB, etc.

private:
    bool load_system_profiles();
    std::string get_profiles_directory();
    bool validate_profile(const std::string& file_path);
    
    std::unordered_map<std::string, ColorProfile> profiles_;
    std::unordered_map<std::string, CalibrationData> calibrations_;
    std::unordered_map<std::string, std::string> printer_profiles_;
    
    std::string default_input_profile_;
    std::string profiles_dir_;
    std::mutex profiles_mutex_;
};

} // namespace AllPress::Color
```

---

## 7. Build System (CMake)

### 7.1 CMakeLists.txt Principal
```cmake
cmake_minimum_required(VERSION 3.20)
project(AllPressCore 
    VERSION 1.0.0
    DESCRIPTION "High-Performance Print Management System"
    LANGUAGES CXX C)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Find packages
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

# CUPS
pkg_check_modules(CUPS REQUIRED cups)

# ImageMagick++
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

# Poppler
pkg_check_modules(POPPLER REQUIRED poppler-cpp)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

# Source files
set(CORE_SOURCES
    src/core/printer_manager.cpp
    src/core/job_queue.cpp
    src/core/color_manager.cpp
    
    src/network/cups_client.cpp
    src/network/ipp_client.cpp
    src/network/network_scanner.cpp
    
    src/conversion/file_processor.cpp
    src/conversion/pdf_processor.cpp
    src/conversion/image_processor.cpp
    src/conversion/office_converter.cpp
    src/conversion/cad_converter.cpp
    
    src/api/http_server.cpp
    src/api/websocket_server.cpp
    src/api/endpoints.cpp
    
    src/database/sqlite_manager.cpp
    src/database/models.cpp
    
    src/utils/logger.cpp
    src/utils/config.cpp
    src/utils/file_utils.cpp
)

# Main executable
add_executable(all_press_server src/main.cpp ${CORE_SOURCES})

# Linking
target_link_libraries(all_press_server
    ${CUPS_LIBRARIES}
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ${ImageMagick_LIBRARIES}
    ${POPPLER_LIBRARIES}
    sqlite3
    pthread
)

# Windows specific
if(WIN32)
    target_link_libraries(all_press_server winspool)
endif()

# Tests
enable_testing()
add_subdirectory(tests)

# Python bindings (optional)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)
if(BUILD_PYTHON_BINDINGS)
    add_subdirectory(python_bindings)
endif()

# Install
install(TARGETS all_press_server DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
```

### 7.2 Conan Dependencies
```python
# conanfile.txt
[requires]
boost/1.82.0
openssl/1.1.1t
nlohmann_json/3.11.2
spdlog/1.12.0
sqlite3/3.42.0
libcurl/8.1.2
imagemagick/7.1.1-15
poppler/23.06.0
libarchive/3.6.2

[generators]
CMakeDeps
CMakeToolchain

[options]
boost:shared=False
openssl:shared=False
sqlite3:shared=False
imagemagick:shared=False
poppler:shared=False

[imports]
bin, *.dll -> ./bin
bin, *.dylib* -> ./bin
lib, *.dylib* -> ./bin
```

---

## 8. Performance Benchmarks Esperados

### 8.1 Descoberta de Impressoras
```
Python/CUPS:     ~2-5 segundos para 254 IPs
C++ Paralelo:    ~0.3-0.8 segundos para 254 IPs
Melhoria:        10-15x mais rápido
```

### 8.2 Processamento de Arquivos
```
Python PIL:      PDF 100MB → ~45-60 segundos  
C++ ImageMagick: PDF 100MB → ~8-15 segundos
Melhoria:        4-6x mais rápido
```

### 8.3 Uso de Memória
```
Python FastAPI:  ~150-300MB base + ~50MB por job
C++ Native:      ~15-30MB base + ~10MB por job
Melhoria:        80-90% menos memória
```

### 8.4 Throughput de Jobs
```
Python:          ~10-20 jobs/minuto simultâneos
C++ Threading:   ~50-100 jobs/minuto simultâneos  
Melhoria:        5x mais throughput
```

---

## 9. Instruções de Build

### 9.1 Dependências do Sistema
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libcups2-dev \
    libboost-all-dev \
    libssl-dev \
    libsqlite3-dev \
    libmagick++-dev \
    libpoppler-cpp-dev \
    libarchive-dev \
    libcurl4-openssl-dev

# macOS
brew install cmake ninja boost openssl sqlite3 imagemagick poppler libarchive curl
```

### 9.2 Build Process
```bash
# Clone e setup
git clone <repo-url> all_press_cpp
cd all_press_cpp

# Install Conan
pip install conan
conan profile detect --force

# Install dependencies
mkdir build && cd build
conan install .. --build=missing

# Configure e build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

# Run tests
ninja test

# Install
sudo ninja install
```

### 9.3 Configuração e Execução
```bash
# Configurar CUPS
sudo systemctl enable cups
sudo systemctl start cups

# Configurar aplicação
mkdir -p /etc/all_press
cp config/all_press.conf /etc/all_press/

# Executar servidor
./all_press_server --config /etc/all_press/all_press.conf

# Ou modo desenvolvimento
./all_press_server --debug --port 8000
```

---

## 10. Integração com Frontend React

O frontend React existente pode ser mantido com ajustes mínimos na API:

### 10.1 API Compatibility Layer
```typescript
// services/api.ts - Manter mesma interface
class PrintAPI {
  private baseURL = 'http://localhost:8000';
  
  async getProinters(): Promise<Printer[]> {
    // Mesma assinatura, mas agora chama C++ backend
    const response = await fetch(`${this.baseURL}/api/printers`);
    return response.json();
  }
  
  async submitPrintJob(job: PrintJobRequest): Promise<PrintJob> {
    // WebSocket para status em tempo real agora é nativo C++
    return fetch(`${this.baseURL}/api/print`, {
      method: 'POST',
      body: JSON.stringify(job)
    }).then(r => r.json());
  }
}
```

### 10.2 WebSocket Integration
```typescript
// hooks/useRealtimeStatus.ts
export function useRealtimeStatus() {
  const [status, setStatus] = useState<PrinterStatus[]>([]);
  
  useEffect(() => {
    // C++ WebSocket server é mais rápido e estável
    const ws = new WebSocket('ws://localhost:8001/ws/printers');
    
    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      setStatus(prev => updatePrinterStatus(prev, data));
    };
    
    return () => ws.close();
  }, []);
  
  return status;
}
```

---

## 11. Vantagens da Implementação C++

### 11.1 Performance
- **10-50x mais rápido** em descoberta de rede
- **4-8x mais rápido** em processamento de arquivos
- **80-90% menos uso** de memória
- **5x mais throughput** de jobs simultâneos
- **Threading real** sem GIL

### 11.2 Robustez
- **APIs nativas** do sistema operacional
- **Recuperação de erros** mais granular
- **Timeouts precisos** em operações
- **Memory safety** com RAII
- **Crash recovery** automático

### 11.3 Escalabilidade
- **Pool de threads** configurável
- **Connection pooling** nativo
- **Load balancing** inteligente
- **Cache hierarchical** de dados
- **Async I/O** real com Boost.Asio

### 11.4 Produção
- **Deployment simples** (binário único)
- **Dependency management** com Conan
- **Cross-platform** (Linux, macOS, Windows)
- **Monitoring nativo** com métricas
- **Hot-reload** de configurações

Esta implementação C++ forneceria ao All Press capacidades de **nível enterprise** com performance adequada para gráficas rápidas de grande volume, mantendo **custo zero** de licenciamento.
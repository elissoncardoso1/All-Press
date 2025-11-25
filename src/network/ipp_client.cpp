#include "network/ipp_client.h"
#include "utils/logger.h"

namespace AllPress {
namespace Network {

IPPClient::IPPClient()
    : http_connection_(nullptr), user_agent_("AllPress/1.0") {}

IPPClient::~IPPClient() {
#if defined(__APPLE__) || defined(__linux__)
  if (http_connection_) {
    httpClose(http_connection_);
  }
#endif
}

std::future<std::vector<IPPPrinter>>
IPPClient::discover_printers_async(const std::string &subnet, int timeout_ms) {

  return std::async(std::launch::async, [this, subnet, timeout_ms]() {
    std::vector<IPPPrinter> results;
    std::mutex results_mutex;

    scan_ip_range_parallel(subnet, results, results_mutex);

    LOG_INFO("IPP discovery found " + std::to_string(results.size()) +
             " printers");
    return results;
  });
}

bool IPPClient::print_document_stream(
    const std::string &printer_uri, const std::string &document_path,
    const std::string &job_name,
    const std::unordered_map<std::string, std::string> &attributes) {
#if defined(__APPLE__) || defined(__linux__)
  LOG_INFO("Printing document to IPP printer: " + printer_uri);

  // Use CUPS IPP API to submit job
  ipp_t *request = ippNewRequest(IPP_OP_PRINT_JOB);
  ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr,
               printer_uri.c_str());
  ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
               nullptr, cupsUser());
  ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "job-name", nullptr,
               job_name.c_str());

  // Add custom attributes
  for (const auto &attr : attributes) {
    ippAddString(request, IPP_TAG_JOB, IPP_TAG_TEXT, attr.first.c_str(),
                 nullptr, attr.second.c_str());
  }

  ipp_t *response = cupsDoFileRequest(
      CUPS_HTTP_DEFAULT, request, printer_uri.c_str(), document_path.c_str());

  bool success = (ippGetStatusCode(response) <= IPP_STATUS_OK_EVENTS_COMPLETE);
  ippDelete(response);

  return success;
#else
  return false;
#endif
}

std::vector<int> IPPClient::get_active_jobs(const std::string &printer_uri) {
  std::vector<int> jobs;
  LOG_INFO("Getting active jobs for: " + printer_uri);
  return jobs;
}

bool IPPClient::cancel_job(const std::string &printer_uri, int job_id) {
  LOG_INFO("Cancelling IPP job " + std::to_string(job_id) + " on " +
           printer_uri);
  return true;
}

std::vector<std::string>
IPPClient::get_supported_formats(const std::string &printer_uri) {
  return {"application/pdf", "image/jpeg", "image/png",
          "application/postscript"};
}

std::vector<std::string>
IPPClient::get_supported_media(const std::string &printer_uri) {
  return {"iso_a4_210x297mm", "na_letter_8.5x11in", "iso_a3_297x420mm"};
}

bool IPPClient::probe_ipp_printer(const std::string &ip, int port) {
  // Try to connect to IPP port
  std::string uri = "ipp://" + ip + ":" + std::to_string(port);

#if defined(__APPLE__) || defined(__linux__)
  http_t *http = httpConnect2(ip.c_str(), port, NULL, AF_UNSPEC,
                              HTTP_ENCRYPTION_IF_REQUESTED, 1, 30000, NULL);
  if (http) {
    httpClose(http);
    return true;
  }
#endif

  return false;
}

void IPPClient::scan_ip_range_parallel(const std::string &subnet,
                                       std::vector<IPPPrinter> &results,
                                       std::mutex &results_mutex) {
  const int num_threads = std::thread::hardware_concurrency();
  const int ips_per_thread = 254 / num_threads;

  std::vector<std::thread> threads;

  for (int t = 0; t < num_threads; ++t) {
    int start = t * ips_per_thread + 1;
    int end = (t == num_threads - 1) ? 254 : (t + 1) * ips_per_thread;

    threads.emplace_back(
        [this, subnet, start, end, &results, &results_mutex]() {
          for (int i = start; i <= end; ++i) {
            std::string ip = subnet + "." + std::to_string(i);

            if (probe_ipp_printer(ip, 631)) {
              std::string uri = "ipp://" + ip + ":631";
              // IPPPrinter printer = get_printer_attributes(uri);
              IPPPrinter printer;
              printer.uri = uri;
              printer.name = "Found Printer";

              if (!printer.name.empty()) {
                std::lock_guard<std::mutex> lock(results_mutex);
                results.push_back(printer);
              }
            }
          }
        });
  }

  for (auto &thread : threads) {
    thread.join();
  }
}

} // namespace Network
} // namespace AllPress

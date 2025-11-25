#pragma once

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef __APPLE__
#include <cups/cups.h>
#elif __linux__
#include <cups/cups.h>
#endif

namespace AllPress {
namespace Network {

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
  std::future<std::vector<IPPPrinter>>
  discover_printers_async(const std::string &subnet = "192.168.1",
                          int timeout_ms = 5000);

  // Operações IPP
  bool print_document_stream(
      const std::string &printer_uri, const std::string &document_path,
      const std::string &job_name,
      const std::unordered_map<std::string, std::string> &attributes);

  inline IPPPrinter get_printer_attributes(const std::string &printer_uri) {
    IPPPrinter printer;
    printer.uri = printer_uri;

#if defined(__APPLE__) || defined(__linux__)
    ipp_t *request = ippNewRequest(IPP_OP_GET_PRINTER_ATTRIBUTES);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 nullptr, printer_uri.c_str());

    ipp_t *response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");

    if (response) {
      ipp_attribute_t *attr =
          ippFindAttribute(response, "printer-name", IPP_TAG_NAME);
      if (attr) {
        printer.name = ippGetString(attr, 0, nullptr);
      }

      attr = ippFindAttribute(response, "printer-make-and-model", IPP_TAG_TEXT);
      if (attr) {
        printer.make_model = ippGetString(attr, 0, nullptr);
      }

      attr = ippFindAttribute(response, "printer-location", IPP_TAG_TEXT);
      if (attr) {
        printer.location = ippGetString(attr, 0, nullptr);
      }

      ippDelete(response);
    }
#endif

    return printer;
  }
  std::vector<int> get_active_jobs(const std::string &printer_uri);
  bool cancel_job(const std::string &printer_uri, int job_id);

  // Capacidades
  std::vector<std::string>
  get_supported_formats(const std::string &printer_uri);
  std::vector<std::string> get_supported_media(const std::string &printer_uri);

private:
  bool probe_ipp_printer(const std::string &ip, int port = 631);

  void scan_ip_range_parallel(const std::string &subnet,
                              std::vector<IPPPrinter> &results,
                              std::mutex &results_mutex);

#if defined(__APPLE__) || defined(__linux__)
  http_t *http_connection_;
#endif
  std::string user_agent_;
};

} // namespace Network
} // namespace AllPress

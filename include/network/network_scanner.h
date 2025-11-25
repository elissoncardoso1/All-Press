#pragma once

#include <string>
#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>

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

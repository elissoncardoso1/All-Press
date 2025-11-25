#include "network/network_scanner.h"
#include "utils/logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

namespace AllPress {
namespace Network {

NetworkScanner::NetworkScanner(size_t thread_count) 
    : thread_count_(thread_count) {
}

NetworkScanner::~NetworkScanner() {
}

std::future<std::vector<NetworkDevice>> NetworkScanner::scan_subnet_async(
    const std::string& subnet,
    const std::vector<int>& ports) {
    
    return std::async(std::launch::async, [this, subnet, ports]() {
        std::vector<NetworkDevice> results;
        std::mutex results_mutex;
        
        const int ips_per_thread = 254 / thread_count_;
        std::vector<std::thread> threads;
        
        for (size_t t = 0; t < thread_count_; ++t) {
            int start = t * ips_per_thread + 1;
            int end = (t == thread_count_ - 1) ? 254 : (t + 1) * ips_per_thread;
            
            threads.emplace_back(&NetworkScanner::scan_ip_worker, this,
                               subnet, start, end, std::ref(ports),
                               std::ref(results), std::ref(results_mutex));
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        LOG_INFO("Network scan found " + std::to_string(results.size()) + " devices");
        return results;
    });
}

std::future<std::vector<NetworkDevice>> NetworkScanner::discover_mdns_devices_async(
    const std::vector<std::string>& service_types) {
    
    return std::async(std::launch::async, [service_types]() {
        std::vector<NetworkDevice> results;
        LOG_INFO("mDNS discovery not yet fully implemented");
        // Implementation would use Avahi or dns-sd
        return results;
    });
}

bool NetworkScanner::ping_host(const std::string& ip, int timeout_ms) {
    // Simple TCP connect test to port 9100 (common printer port)
    return check_port(ip, 9100, timeout_ms);
}

bool NetworkScanner::check_port(const std::string& ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return false;
    }
    
    // Set non-blocking
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    
    bool connected = false;
    if (select(sock + 1, nullptr, &fdset, nullptr, &tv) > 0) {
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
        connected = (error == 0);
    }
    
    close(sock);
    return connected;
}

std::string NetworkScanner::resolve_hostname(const std::string& ip) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    
    char hostname[NI_MAXHOST];
    if (getnameinfo((struct sockaddr*)&addr, sizeof(addr), 
                    hostname, NI_MAXHOST, nullptr, 0, 0) == 0) {
        return std::string(hostname);
    }
    
    return ip;
}

void NetworkScanner::scan_ip_worker(const std::string& base_ip, int start_host, int end_host,
                                   const std::vector<int>& ports,
                                   std::vector<NetworkDevice>& results,
                                   std::mutex& results_mutex) {
    for (int host = start_host; host <= end_host; ++host) {
        std::string ip = base_ip + "." + std::to_string(host);
        
        NetworkDevice device;
        device.ip = ip;
        
        for (int port : ports) {
            if (check_port(ip, port, 100)) {
                device.open_ports.push_back(port);
                
                if (identify_printer_service(ip, port)) {
                    device.is_printer = true;
                    
                    if (port == 631) device.printer_type = "IPP";
                    else if (port == 9100) device.printer_type = "Raw";
                    else if (port == 515) device.printer_type = "LPD";
                }
            }
        }
        
        if (!device.open_ports.empty()) {
            device.hostname = resolve_hostname(ip);
            
            std::lock_guard<std::mutex> lock(results_mutex);
            results.push_back(device);
        }
    }
}

bool NetworkScanner::identify_printer_service(const std::string& ip, int port) {
    // Ports commonly used by printers
    return (port == 631 || port == 9100 || port == 515);
}

} // namespace Network
} // namespace AllPress

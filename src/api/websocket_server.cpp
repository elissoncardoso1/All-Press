#include "utils/logger.h"
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <iomanip>
#include <sstream>

namespace AllPress {
namespace API {

class WebSocketServerImpl {
public:
    WebSocketServerImpl(int port) : port_(port), running_(false), server_fd_(-1) {
        LOG_INFO("WebSocket Server initialized on port " + std::to_string(port));
    }
    
    ~WebSocketServerImpl() {
        stop();
    }
    
    void start() {
        if (running_) return;
        
        running_ = true;
        server_thread_ = std::thread(&WebSocketServerImpl::run_server, this);
        LOG_INFO("WebSocket Server starting on port " + std::to_string(port_));
    }
    
    void stop() {
        if (!running_) return;
        
        running_ = false;
        
        // Close all client connections
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (int fd : clients_) {
            close(fd);
        }
        clients_.clear();
        
        if (server_fd_ >= 0) {
            close(server_fd_);
            server_fd_ = -1;
        }
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        LOG_INFO("WebSocket Server stopped");
    }
    
    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto it = clients_.begin(); it != clients_.end();) {
            if (send_message(*it, message)) {
                ++it;
            } else {
                close(*it);
                it = clients_.erase(it);
            }
        }
    }
    
private:
    void run_server() {
        struct sockaddr_in address;
        int opt = 1;
        
        // Create socket
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            LOG_ERROR("Failed to create WebSocket socket");
            return;
        }
        
        // Set socket options
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        // Bind socket
        if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
            LOG_ERROR("Failed to bind WebSocket to port " + std::to_string(port_));
            close(server_fd_);
            return;
        }
        
        // Listen
        if (listen(server_fd_, 10) < 0) {
            LOG_ERROR("Failed to listen on WebSocket socket");
            close(server_fd_);
            return;
        }
        
        LOG_INFO("WebSocket Server listening on port " + std::to_string(port_));
        
        // Accept connections
        while (running_) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (running_) {
                    LOG_WARNING("Failed to accept WebSocket connection");
                }
                continue;
            }
            
            // Handle connection in separate thread
            std::thread([this, client_fd]() {
                handle_connection(client_fd);
            }).detach();
        }
    }
    
    void handle_connection(int client_fd) {
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read <= 0) {
            close(client_fd);
            return;
        }
        
        std::string request(buffer);
        
        // Check if this is a WebSocket upgrade request
        bool is_ws_request = (request.find("Upgrade: websocket") != std::string::npos ||
                              request.find("upgrade: websocket") != std::string::npos);
        
        if (is_ws_request) {
            // Extract Sec-WebSocket-Key
            std::string ws_key;
            size_t key_pos = request.find("Sec-WebSocket-Key:");
            if (key_pos == std::string::npos) {
                key_pos = request.find("sec-websocket-key:");
            }
            
            if (key_pos != std::string::npos) {
                size_t key_start = request.find(": ", key_pos) + 2;
                size_t key_end = request.find("\r\n", key_start);
                if (key_end != std::string::npos) {
                    ws_key = request.substr(key_start, key_end - key_start);
                }
            }
            
            if (!ws_key.empty()) {
                // Perform WebSocket handshake
                std::string accept_key = generate_accept_key(ws_key);
                std::string response = create_handshake_response(accept_key);
                
                send(client_fd, response.c_str(), response.length(), 0);
                
                // Add client to list
                {
                    std::lock_guard<std::mutex> lock(clients_mutex_);
                    clients_.push_back(client_fd);
                }
                
                LOG_INFO("WebSocket client connected");
                
                // Send welcome message
                send_message(client_fd, "{\"type\":\"connected\",\"message\":\"WebSocket connected\"}");
                
                // Keep connection alive and handle messages
                keep_alive(client_fd);
            } else {
                close(client_fd);
            }
        } else {
            close(client_fd);
        }
    }
    
    void keep_alive(int client_fd) {
        char buffer[4096];
        while (running_) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(client_fd, &read_fds);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int activity = select(client_fd + 1, &read_fds, nullptr, nullptr, &timeout);
            
            if (activity < 0 && running_) {
                break;
            }
            
            if (FD_ISSET(client_fd, &read_fds)) {
                ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
                if (bytes_read <= 0) {
                    break;
                }
                // Echo back or handle message
            }
        }
        
        // Remove client
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.erase(std::remove(clients_.begin(), clients_.end(), client_fd), clients_.end());
        }
        close(client_fd);
        LOG_INFO("WebSocket client disconnected");
    }
    
    std::string generate_accept_key(const std::string& key) {
        const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::string combined = key + magic;
        
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
        
        // Base64 encode using OpenSSL
        BIO* b64 = BIO_new(BIO_f_base64());
        BIO* mem = BIO_new(BIO_s_mem());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        b64 = BIO_push(b64, mem);
        
        BIO_write(b64, hash, SHA_DIGEST_LENGTH);
        BIO_flush(b64);
        
        BUF_MEM* buffer_ptr;
        BIO_get_mem_ptr(b64, &buffer_ptr);
        
        std::string encoded(buffer_ptr->data, buffer_ptr->length);
        // Remove any trailing newlines
        encoded.erase(std::remove(encoded.begin(), encoded.end(), '\n'), encoded.end());
        encoded.erase(std::remove(encoded.begin(), encoded.end(), '\r'), encoded.end());
        
        BIO_free_all(b64);
        
        return encoded;
    }
    
    std::string create_handshake_response(const std::string& accept_key) {
        std::ostringstream oss;
        oss << "HTTP/1.1 101 Switching Protocols\r\n";
        oss << "Upgrade: websocket\r\n";
        oss << "Connection: Upgrade\r\n";
        oss << "Sec-WebSocket-Accept: " << accept_key << "\r\n";
        oss << "\r\n";
        return oss.str();
    }
    
    bool send_message(int client_fd, const std::string& message) {
        // Simple WebSocket frame (unmasked, text frame)
        std::vector<unsigned char> frame;
        
        // FIN + opcode (0x81 = text frame)
        frame.push_back(0x81);
        
        // Payload length
        size_t len = message.length();
        if (len < 126) {
            frame.push_back(len);
        } else if (len < 65536) {
            frame.push_back(126);
            frame.push_back((len >> 8) & 0xFF);
            frame.push_back(len & 0xFF);
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; i--) {
                frame.push_back((len >> (i * 8)) & 0xFF);
            }
        }
        
        // Payload
        for (char c : message) {
            frame.push_back(c);
        }
        
        ssize_t sent = send(client_fd, frame.data(), frame.size(), 0);
        return sent > 0;
    }
    
    int port_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    int server_fd_;
    std::vector<int> clients_;
    std::mutex clients_mutex_;
};

// Global instance
static std::unique_ptr<WebSocketServerImpl> g_ws_server;

void init_websocket_server(int port) {
    g_ws_server = std::make_unique<WebSocketServerImpl>(port);
    g_ws_server->start();
}

void stop_websocket_server() {
    if (g_ws_server) {
        g_ws_server->stop();
        g_ws_server.reset();
    }
}

void broadcast_websocket_message(const std::string& message) {
    if (g_ws_server) {
        g_ws_server->broadcast(message);
    }
}

} // namespace API
} // namespace AllPress

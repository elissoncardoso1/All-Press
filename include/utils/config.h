#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

namespace AllPress::Utils {

class Config {
public:
    static Config& instance();
    
    bool load_from_file(const std::string& file_path);
    bool save_to_file(const std::string& file_path);
    
    std::string get_string(const std::string& key, const std::string& default_value = "");
    int get_int(const std::string& key, int default_value = 0);
    bool get_bool(const std::string& key, bool default_value = false);
    double get_double(const std::string& key, double default_value = 0.0);
    
    void set_string(const std::string& key, const std::string& value);
    void set_int(const std::string& key, int value);
    void set_bool(const std::string& key, bool value);
    void set_double(const std::string& key, double value);
    
    bool has_key(const std::string& key);
    
    // JSON methods for API
    std::string to_json() const;
    bool from_json(const std::string& json_str);
    std::string get_all_settings_json() const;
    
private:
    // Make mutex mutable for const methods
    mutable std::mutex config_mutex_;

private:
    Config();
    ~Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    std::unordered_map<std::string, std::string> config_map_;
    std::string config_file_path_;
};

} // namespace AllPress::Utils

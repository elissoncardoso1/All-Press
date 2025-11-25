#include "utils/config.h"
#include "utils/logger.h"
#include <fstream>
#include <sstream>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AllPress {
namespace Utils {

Config& Config::instance() {
    static Config instance;
    return instance;
}

Config::Config() {
}

Config::~Config() {
}

bool Config::load_from_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open config file: " + file_path);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_map_.clear();
    config_file_path_ = file_path;
    
    std::string line;
    std::string current_section;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Check for section headers [section]
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Parse key=value pairs
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Add section prefix if present
            if (!current_section.empty()) {
                key = current_section + "." + key;
            }
            
            config_map_[key] = value;
        }
    }
    
    LOG_INFO("Loaded configuration from: " + file_path);
    return true;
}

bool Config::save_to_file(const std::string& file_path) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to write config file: " + file_path);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // Group by section
    std::map<std::string, std::map<std::string, std::string>> sections;
    std::vector<std::pair<std::string, std::string>> top_level;
    
    for (const auto& pair : config_map_) {
        size_t dot_pos = pair.first.find('.');
        if (dot_pos != std::string::npos) {
            std::string section = pair.first.substr(0, dot_pos);
            std::string key = pair.first.substr(dot_pos + 1);
            sections[section][key] = pair.second;
        } else {
            top_level.push_back(pair);
        }
    }
    
    // Write sections
    for (const auto& section : sections) {
        file << "[" << section.first << "]" << std::endl;
        for (const auto& pair : section.second) {
            file << pair.first << "=" << pair.second << std::endl;
        }
        file << std::endl;
    }
    
    // Write top-level keys
    for (const auto& pair : top_level) {
        file << pair.first << "=" << pair.second << std::endl;
    }
    
    if (!file_path.empty()) {
        config_file_path_ = file_path;
    }
    
    LOG_INFO("Saved configuration to: " + (file_path.empty() ? config_file_path_ : file_path));
    return true;
}

std::string Config::get_string(const std::string& key, const std::string& default_value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        return it->second;
    }
    return default_value;
}

int Config::get_int(const std::string& key, int default_value) {
    std::string value = get_string(key, "");
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

bool Config::get_bool(const std::string& key, bool default_value) {
    std::string value = get_string(key, "");
    if (value.empty()) {
        return default_value;
    }
    
    return (value == "true" || value == "1" || value == "yes" || value == "on");
}

double Config::get_double(const std::string& key, double default_value) {
    std::string value = get_string(key, "");
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stod(value);
    } catch (...) {
        return default_value;
    }
}

void Config::set_string(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_map_[key] = value;
}

void Config::set_int(const std::string& key, int value) {
    set_string(key, std::to_string(value));
}

void Config::set_bool(const std::string& key, bool value) {
    set_string(key, value ? "true" : "false");
}

void Config::set_double(const std::string& key, double value) {
    set_string(key, std::to_string(value));
}

bool Config::has_key(const std::string& key) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_map_.find(key) != config_map_.end();
}

std::string Config::to_json() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    json j;
    
    for (const auto& pair : config_map_) {
        j[pair.first] = pair.second;
    }
    
    return j.dump(2);
}

bool Config::from_json(const std::string& json_str) {
    try {
        json j = json::parse(json_str);
        std::lock_guard<std::mutex> lock(config_mutex_);
        
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.value().is_string()) {
                config_map_[it.key()] = it.value().get<std::string>();
            } else if (it.value().is_number_integer()) {
                config_map_[it.key()] = std::to_string(it.value().get<int>());
            } else if (it.value().is_boolean()) {
                config_map_[it.key()] = it.value().get<bool>() ? "true" : "false";
            } else if (it.value().is_number_float()) {
                config_map_[it.key()] = std::to_string(it.value().get<double>());
            }
        }
        
        LOG_INFO("Loaded configuration from JSON");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse JSON configuration: " + std::string(e.what()));
        return false;
    }
}

std::string Config::get_all_settings_json() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    json j;
    
    // Organize by sections
    std::map<std::string, json> sections;
    
    for (const auto& pair : config_map_) {
        size_t dot_pos = pair.first.find('.');
        if (dot_pos != std::string::npos) {
            std::string section = pair.first.substr(0, dot_pos);
            std::string key = pair.first.substr(dot_pos + 1);
            
            if (sections.find(section) == sections.end()) {
                sections[section] = json::object();
            }
            
            // Try to parse as appropriate type
            try {
                if (pair.second == "true" || pair.second == "false") {
                    sections[section][key] = (pair.second == "true");
                } else {
                    try {
                        int int_val = std::stoi(pair.second);
                        sections[section][key] = int_val;
                    } catch (...) {
                        try {
                            double double_val = std::stod(pair.second);
                            sections[section][key] = double_val;
                        } catch (...) {
                            sections[section][key] = pair.second;
                        }
                    }
                }
            } catch (...) {
                sections[section][key] = pair.second;
            }
        } else {
            j[pair.first] = pair.second;
        }
    }
    
    // Add sections to main json
    for (const auto& section : sections) {
        j[section.first] = section.second;
    }
    
    return j.dump(2);
}

} // namespace Utils
} // namespace AllPress

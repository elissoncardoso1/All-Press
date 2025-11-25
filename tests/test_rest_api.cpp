#include <gtest/gtest.h>
#include "utils/config.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fs = std::filesystem;

class RESTAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary config file
        test_config_path_ = "/tmp/test_all_press.conf";
        
        // Clean up any existing test config
        if (fs::exists(test_config_path_)) {
            fs::remove(test_config_path_);
        }
        
        // Create test config file
        std::ofstream config_file(test_config_path_);
        config_file << "[server]\n";
        config_file << "port=8000\n";
        config_file << "ws_port=8001\n";
        config_file << "\n";
        config_file << "[queue]\n";
        config_file << "max_workers=4\n";
        config_file << "\n";
        config_file << "[printer]\n";
        config_file << "auto_discover=true\n";
        config_file.close();
        
        // Load test config
        AllPress::Utils::Config::instance().load_from_file(test_config_path_);
    }
    
    void TearDown() override {
        // Clean up test config file
        if (fs::exists(test_config_path_)) {
            fs::remove(test_config_path_);
        }
    }
    
    std::string test_config_path_;
};

// Test Config JSON serialization
TEST_F(RESTAPITest, ConfigToJSON) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    std::string json_str = config.get_all_settings_json();
    ASSERT_FALSE(json_str.empty());
    
    // Parse JSON to verify it's valid
    json j = json::parse(json_str);
    ASSERT_TRUE(j.is_object());
    
    // Verify server section exists
    ASSERT_TRUE(j.contains("server"));
    ASSERT_TRUE(j["server"].is_object());
    ASSERT_EQ(j["server"]["port"], 8000);
    ASSERT_EQ(j["server"]["ws_port"], 8001);
}

// Test Config JSON deserialization
TEST_F(RESTAPITest, ConfigFromJSON) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    // Create test JSON
    json test_json = {
        {"server", {
            {"port", 9000},
            {"ws_port", 9001}
        }},
        {"queue", {
            {"max_workers", 8}
        }}
    };
    
    // Load from JSON
    bool result = config.from_json(test_json.dump());
    ASSERT_TRUE(result);
    
    // Verify values were set
    ASSERT_EQ(config.get_int("server.port"), 9000);
    ASSERT_EQ(config.get_int("server.ws_port"), 9001);
    ASSERT_EQ(config.get_int("queue.max_workers"), 8);
}

// Test Config save to file
TEST_F(RESTAPITest, ConfigSaveToFile) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    // Modify a setting
    config.set_int("server.port", 8080);
    
    // Save to file
    std::string save_path = "/tmp/test_save_all_press.conf";
    bool result = config.save_to_file(save_path);
    ASSERT_TRUE(result);
    ASSERT_TRUE(fs::exists(save_path));
    
    // Reload and verify
    AllPress::Utils::Config new_config;
    new_config.load_from_file(save_path);
    ASSERT_EQ(new_config.get_int("server.port"), 8080);
    
    // Clean up
    if (fs::exists(save_path)) {
        fs::remove(save_path);
    }
}

// Test Config get all settings
TEST_F(RESTAPITest, ConfigGetAllSettings) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    std::string json_str = config.get_all_settings_json();
    ASSERT_FALSE(json_str.empty());
    
    json j = json::parse(json_str);
    
    // Verify structure
    ASSERT_TRUE(j.is_object());
    
    // Verify sections exist
    ASSERT_TRUE(j.contains("server"));
    ASSERT_TRUE(j.contains("queue"));
    ASSERT_TRUE(j.contains("printer"));
}

// Test Config with invalid JSON
TEST_F(RESTAPITest, ConfigInvalidJSON) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    // Try to load invalid JSON
    bool result = config.from_json("invalid json {");
    ASSERT_FALSE(result);
}

// Test Config with nested objects
TEST_F(RESTAPITest, ConfigNestedObjects) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    json test_json = {
        {"server", {
            {"port", 7000},
            {"host", "0.0.0.0"}
        }},
        {"logging", {
            {"level", "DEBUG"},
            {"file", "/tmp/test.log"}
        }}
    };
    
    bool result = config.from_json(test_json.dump());
    ASSERT_TRUE(result);
    
    ASSERT_EQ(config.get_int("server.port"), 7000);
    ASSERT_EQ(config.get_string("server.host"), "0.0.0.0");
    ASSERT_EQ(config.get_string("logging.level"), "DEBUG");
    ASSERT_EQ(config.get_string("logging.file"), "/tmp/test.log");
}

// Test Config boolean values
TEST_F(RESTAPITest, ConfigBooleanValues) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    json test_json = {
        {"printer", {
            {"auto_discover", true},
            {"enabled", false}
        }}
    };
    
    bool result = config.from_json(test_json.dump());
    ASSERT_TRUE(result);
    
    ASSERT_TRUE(config.get_bool("printer.auto_discover"));
    ASSERT_FALSE(config.get_bool("printer.enabled"));
}

// Test Config numeric values
TEST_F(RESTAPITest, ConfigNumericValues) {
    AllPress::Utils::Config& config = AllPress::Utils::Config::instance();
    
    json test_json = {
        {"queue", {
            {"max_workers", 16},
            {"timeout", 30.5}
        }}
    };
    
    bool result = config.from_json(test_json.dump());
    ASSERT_TRUE(result);
    
    ASSERT_EQ(config.get_int("queue.max_workers"), 16);
    ASSERT_DOUBLE_EQ(config.get_double("queue.timeout"), 30.5);
}


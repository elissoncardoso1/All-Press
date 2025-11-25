#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>

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

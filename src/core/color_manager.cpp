#include "core/color_manager.h"
#include "utils/logger.h"
#include "utils/file_utils.h"
#include <algorithm>

namespace AllPress {
namespace Color {

ColorManager::ColorManager() {
    profiles_dir_ = "/usr/share/color/icc";
}

ColorManager::~ColorManager() {
}

bool ColorManager::initialize() {
    LOG_INFO("Initializing ColorManager");
    
    if (!load_system_profiles()) {
        LOG_WARNING("Failed to load system color profiles");
    }
    
    return install_standard_profiles();
}

bool ColorManager::load_profile(const std::string& file_path, const std::string& name) {
    if (!Utils::FileUtils::file_exists(file_path)) {
        LOG_ERROR("Profile file not found: " + file_path);
        return false;
    }
    
    if (!validate_profile(file_path)) {
        LOG_ERROR("Invalid color profile: " + file_path);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    ColorProfile profile;
    profile.name = name.empty() ? Utils::FileUtils::get_filename(file_path) : name;
    profile.file_path = file_path;
    profile.description = "Custom color profile";
    profile.device_class = "output";
    profile.color_space = "RGB";
    
    profiles_[profile.name] = profile;
    
    LOG_INFO("Loaded color profile: " + profile.name);
    return true;
}

bool ColorManager::remove_profile(const std::string& name) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    auto it = profiles_.find(name);
    if (it != profiles_.end()) {
        profiles_.erase(it);
        LOG_INFO("Removed color profile: " + name);
        return true;
    }
    return false;
}

std::vector<ColorProfile> ColorManager::get_available_profiles() {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    std::vector<ColorProfile> result;
    for (const auto& pair : profiles_) {
        result.push_back(pair.second);
    }
    return result;
}

ColorProfile ColorManager::get_profile(const std::string& name) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    auto it = profiles_.find(name);
    if (it != profiles_.end()) {
        return it->second;
    }
    return ColorProfile();
}

bool ColorManager::convert_image_colors(const std::string& input_path,
                                       const std::string& output_path,
                                       const std::string& source_profile,
                                       const std::string& target_profile,
                                       RenderingIntent intent) {
    LOG_INFO("Converting image colors from " + source_profile + " to " + target_profile);
    // Implementation would use ImageMagick or LittleCMS
    return true;
}

bool ColorManager::apply_color_profile_to_pdf(const std::string& pdf_path,
                                             const std::string& output_path,
                                             const std::string& target_profile) {
    LOG_INFO("Applying color profile to PDF: " + target_profile);
    // Implementation would use Poppler or Ghostscript
    return true;
}

bool ColorManager::calibrate_printer(const std::string& printer_name,
                                    const std::string& test_pattern_path) {
    LOG_INFO("Calibrating printer: " + printer_name);
    
    CalibrationData calibration;
    calibration.printer_name = printer_name;
    calibration.calibrated_at = std::chrono::system_clock::now();
    calibration.gamma = 1.0;
    calibration.is_valid = true;
    
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    calibrations_[printer_name] = calibration;
    
    return true;
}

CalibrationData ColorManager::get_printer_calibration(const std::string& printer_name) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    auto it = calibrations_.find(printer_name);
    if (it != calibrations_.end()) {
        return it->second;
    }
    return CalibrationData();
}

bool ColorManager::apply_calibration(const std::string& printer_name,
                                    const std::string& document_path) {
    LOG_INFO("Applying calibration for printer: " + printer_name);
    return true;
}

void ColorManager::set_default_input_profile(const std::string& profile_name) {
    default_input_profile_ = profile_name;
    LOG_INFO("Default input profile set to: " + profile_name);
}

void ColorManager::set_default_output_profile(const std::string& printer_name,
                                             const std::string& profile_name) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    printer_profiles_[printer_name] = profile_name;
    LOG_INFO("Default output profile for " + printer_name + " set to: " + profile_name);
}

std::string ColorManager::get_default_profile_for_printer(const std::string& printer_name) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    auto it = printer_profiles_.find(printer_name);
    if (it != printer_profiles_.end()) {
        return it->second;
    }
    return "sRGB";
}

bool ColorManager::install_standard_profiles() {
    LOG_INFO("Installing standard color profiles");
    
    // Create default sRGB profile entry
    ColorProfile srgb;
    srgb.name = "sRGB";
    srgb.description = "Standard RGB color space";
    srgb.device_class = "output";
    srgb.color_space = "RGB";
    srgb.is_default = true;
    
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    profiles_["sRGB"] = srgb;
    
    return true;
}

bool ColorManager::load_system_profiles() {
    // Load profiles from system directory
    if (!Utils::FileUtils::directory_exists(profiles_dir_)) {
        return false;
    }
    
    // Implementation would scan directory for .icc and .icm files
    return true;
}

std::string ColorManager::get_profiles_directory() {
    return profiles_dir_;
}

bool ColorManager::validate_profile(const std::string& file_path) {
    // Basic validation - check file exists and has ICC extension
    if (!Utils::FileUtils::file_exists(file_path)) {
        return false;
    }
    
    std::string ext = Utils::FileUtils::get_file_extension(file_path);
    return (ext == ".icc" || ext == ".icm");
}

} // namespace Color
} // namespace AllPress

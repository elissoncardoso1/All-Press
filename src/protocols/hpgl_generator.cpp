#include "protocols/hpgl_generator.h"
#include <sstream>
#include <cmath>

namespace all_press {
namespace protocols {

HPGLGenerator::HPGLGenerator(bool use_hpgl2) : use_hpgl2_(use_hpgl2) {
    // Carregar capabilities de HP
    capabilities_.vendor = PlotterVendor::HP;
    capabilities_.supported_sizes = {
        MediaSize::A0, MediaSize::A1, MediaSize::A2,
        MediaSize::A3, MediaSize::A4, MediaSize::LETTER
    };
    capabilities_.supported_resolutions = {300, 600, 1200};
    capabilities_.supported_colors = {
        ColorMode::MONOCHROME, ColorMode::COLOR
    };
    capabilities_.supports_duplex = false;
    capabilities_.supports_booklet = false;
    capabilities_.max_paper_width_mm = 1118;  // 44 polegadas
    capabilities_.max_paper_height_mm = 1600; // ou comprimento de roll
}

std::vector<uint8_t> HPGLGenerator::generate_header(
    const PlotterCapabilities& caps,
    MediaSize media_size,
    ColorMode color_mode,
    int dpi) {
    
    std::string header;
    
    // Inicialização do plotter
    header += "\x1B" ".@";  // Reset (ESC .@)
    header += "ES";         // Enter Setup Mode (HPGL2)
    
    if (use_hpgl2_) {
        header += "\x1B" "%0B";  // Enter HPGL2 mode (ESC %0B)
    }
    
    // Page Setup
    header += "PU0,0;";  // Pen Up at origin
    header += "PA0,0;";  // Plot Absolute at origin
    
    // Media Configuration
    if (media_size_map_.count(media_size) > 0) {
        std::string media_cmd = "PM" + media_size_map_[media_size] + ";";
        header += media_cmd;
    }
    
    // Resolution
    header += "PS" + std::to_string(dpi) + ";";
    
    // Color Mode (se HPGL2)
    if (use_hpgl2_ && color_mode == ColorMode::COLOR) {
        header += "MC3;";  // Multi-color mode (3 pens)
    }
    
    // Incrementar pen number para desenho
    header += "SP1;";  // Select Pen 1
    
    std::vector<uint8_t> result(header.begin(), header.end());
    return result;
}

std::vector<uint8_t> HPGLGenerator::generate_page(
    const std::vector<uint8_t>& raster_data,
    int width,
    int height,
    int dpi) {
    
    std::vector<uint8_t> result;
    
    // Converter raster para comandos HPGL
    // Aqui você processaria os dados raster e geraria
    // comandos PA (Plot Absolute) e PU/PD (Pen Up/Down)
    
    // Para simplificar, retornamos os dados brutos com header
    result.insert(result.end(), raster_data.begin(), raster_data.end());
    
    return result;
}

std::vector<uint8_t> HPGLGenerator::generate_footer() {
    std::string footer;
    
    footer += "PU;";           // Pen Up
    footer += "\x1B" "E";      // Exit plot mode (ESC E)
    
    if (use_hpgl2_) {
        footer += "\x1B" "%0A"; // Exit HPGL2 mode (ESC %0A)
    }
    
    footer += "\x1B" ".@";     // Reset (ESC .@)
    
    std::vector<uint8_t> result(footer.begin(), footer.end());
    return result;
}

bool HPGLGenerator::validate_media_size(MediaSize size) const {
    return media_size_map_.count(size) > 0;
}

bool HPGLGenerator::validate_resolution(int dpi) const {
    return resolution_map_.count(dpi) > 0;
}

bool HPGLGenerator::validate_color_mode(ColorMode mode) const {
    return (mode == ColorMode::MONOCHROME || 
            (use_hpgl2_ && mode == ColorMode::COLOR));
}

std::string HPGLGenerator::get_protocol_name() const {
    return use_hpgl2_ ? "HPGL2" : "HPGL";
}

PlotterCapabilities HPGLGenerator::get_capabilities() const {
    return capabilities_;
}

std::vector<uint8_t> HPGLGenerator::optimize_for_vendor(
    const std::vector<uint8_t>& data) {
    // Otimizações específicas de HP
    // Ex: compressão de dados, buffering
    return data;
}

bool HPGLGenerator::needs_preprocessing() const {
    return true;  // HPGL requer conversão de raster
}

}  // namespace protocols
}  // namespace all_press


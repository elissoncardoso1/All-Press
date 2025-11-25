#include "protocols/postscript_generator.h"
#include <sstream>
#include <ctime>

namespace all_press {
namespace protocols {

PostScriptGenerator::PostScriptGenerator(PlotterVendor vendor) 
    : target_vendor_(vendor) {
    
    capabilities_.vendor = vendor;
    capabilities_.model = (vendor == PlotterVendor::CANON) 
        ? "imagePROGRAF" : "SureColor";
    
    capabilities_.supported_sizes = {
        MediaSize::A0, MediaSize::A1, MediaSize::A2,
        MediaSize::A3, MediaSize::A4, MediaSize::LETTER,
        MediaSize::LEGAL, MediaSize::TABLOID
    };
    
    capabilities_.supported_resolutions = {
        300, 600, 720, 1200
    };
    
    capabilities_.supported_colors = {
        ColorMode::MONOCHROME, ColorMode::COLOR
    };
    
    capabilities_.supports_duplex = false;
    capabilities_.supports_booklet = false;
    capabilities_.max_paper_width_mm = 1118;
    capabilities_.max_paper_height_mm = 1600;
}

std::vector<uint8_t> PostScriptGenerator::generate_header(
    const PlotterCapabilities& caps,
    MediaSize media_size,
    ColorMode color_mode,
    int dpi) {
    
    std::stringstream ps;
    
    // PostScript Header
    ps << "%!PS-Adobe-3.0\n";
    ps << "%%Creator: All Press C++\n";
    ps << "%%CreationDate: " << std::time(nullptr) << "\n";
    
    // Media Configuration
    if (media_dimensions_.count(media_size) > 0) {
        auto [width, height] = media_dimensions_[media_size];
        ps << "<<\n";
        ps << "  /PageSize [" << width << " " << height << "]\n";
        ps << "  /MediaClass (plain)\n";
        
        if (color_mode == ColorMode::COLOR) {
            ps << "  /ColorModel /DeviceRGB\n";
        } else {
            ps << "  /ColorModel /DeviceGray\n";
        }
        
        ps << ">> setpagedevice\n\n";
    }
    
    // Resolution
    ps << "currentdevice /HWResolution\n";
    ps << "[" << dpi << " " << dpi << "] def\n\n";
    
    // Vendor-specific optimizations
    if (target_vendor_ == PlotterVendor::CANON) {
        ps << "% Canon imagePROGRAF settings\n";
        ps << "<< /ColorRenderingType 1 >> setuserparams\n";
    } else if (target_vendor_ == PlotterVendor::EPSON) {
        ps << "% Epson SureColor settings\n";
        ps << "<< /Optimize true >> setuserparams\n";
    }
    
    ps << "%%EndProlog\n\n";
    
    std::string result_str = ps.str();
    std::vector<uint8_t> result(result_str.begin(), result_str.end());
    return result;
}

std::vector<uint8_t> PostScriptGenerator::generate_page(
    const std::vector<uint8_t>& raster_data,
    int width,
    int height,
    int dpi) {
    
    std::stringstream ps;
    
    ps << "gsave\n";
    ps << width << " " << height << " scale\n";
    ps << "currentfile /DCTDecode filter\n";
    ps << "image\n";
    
    std::string result_str = ps.str();
    std::vector<uint8_t> result(result_str.begin(), result_str.end());
    result.insert(result.end(), raster_data.begin(), raster_data.end());
    
    return result;
}

std::vector<uint8_t> PostScriptGenerator::generate_footer() {
    std::string footer = "grestore\nshowpage\n%%EOF\n";
    std::vector<uint8_t> result(footer.begin(), footer.end());
    return result;
}

bool PostScriptGenerator::validate_media_size(MediaSize size) const {
    return media_dimensions_.count(size) > 0;
}

bool PostScriptGenerator::validate_resolution(int dpi) const {
    return (dpi == 300 || dpi == 600 || dpi == 720 || dpi == 1200);
}

bool PostScriptGenerator::validate_color_mode(ColorMode mode) const {
    return (mode == ColorMode::MONOCHROME || mode == ColorMode::COLOR);
}

std::string PostScriptGenerator::get_protocol_name() const {
    return "PostScript";
}

PlotterCapabilities PostScriptGenerator::get_capabilities() const {
    return capabilities_;
}

std::vector<uint8_t> PostScriptGenerator::optimize_for_vendor(
    const std::vector<uint8_t>& data) {
    
    if (target_vendor_ == PlotterVendor::CANON) {
        // Otimizações para Canon
        // Ex: Adobe CMYK rendering
    } else if (target_vendor_ == PlotterVendor::EPSON) {
        // Otimizações para Epson
        // Ex: UltraChrome ink optimization
    }
    
    return data;
}

bool PostScriptGenerator::needs_preprocessing() const {
    return false;  // PostScript é mais universal
}

}  // namespace protocols
}  // namespace all_press


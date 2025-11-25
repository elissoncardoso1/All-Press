#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace all_press {
namespace protocols {

enum class PlotterVendor {
    HP,
    CANON,
    EPSON,
    GENERIC
};

enum class MediaSize {
    A0, A1, A2, A3, A4, A5,
    B0, B1, B2, B3, B4, B5,
    LETTER, LEGAL, TABLOID,
    CUSTOM
};

enum class ColorMode {
    MONOCHROME,
    COLOR,
    RGB,
    CMYK
};

struct PlotterCapabilities {
    PlotterVendor vendor;
    std::string model;
    std::vector<MediaSize> supported_sizes;
    std::vector<int> supported_resolutions;  // em DPI
    std::vector<ColorMode> supported_colors;
    bool supports_duplex;
    bool supports_booklet;
    float max_paper_width_mm;  // para custom sizes
    float max_paper_height_mm;
    std::map<std::string, std::string> custom_attributes;
};

class PlotterProtocolBase {
public:
    virtual ~PlotterProtocolBase() = default;

    // Geração de comandos
    virtual std::vector<uint8_t> generate_header(
        const PlotterCapabilities& caps,
        MediaSize media_size,
        ColorMode color_mode,
        int dpi) = 0;

    virtual std::vector<uint8_t> generate_page(
        const std::vector<uint8_t>& raster_data,
        int width,
        int height,
        int dpi) = 0;

    virtual std::vector<uint8_t> generate_footer() = 0;

    // Validação de compatibilidade
    virtual bool validate_media_size(MediaSize size) const = 0;
    virtual bool validate_resolution(int dpi) const = 0;
    virtual bool validate_color_mode(ColorMode mode) const = 0;

    // Metadados
    virtual std::string get_protocol_name() const = 0;
    virtual PlotterCapabilities get_capabilities() const = 0;

    // Otimizações específicas
    virtual std::vector<uint8_t> optimize_for_vendor(
        const std::vector<uint8_t>& data) = 0;

    virtual bool needs_preprocessing() const = 0;
};

}  // namespace protocols
}  // namespace all_press


#pragma once

#include "plotter_protocol_base.h"
#include <sstream>
#include <cmath>

namespace all_press {
namespace protocols {

class HPGLGenerator : public PlotterProtocolBase {
private:
    PlotterCapabilities capabilities_;
    bool use_hpgl2_;  // true para HPGL2, false para HPGL
    
    // Mapeamento de tamanho de papel HPGL
    std::map<MediaSize, std::string> media_size_map_ = {
        {MediaSize::A0, "ROL"},      // Roll
        {MediaSize::A1, "A1P"},
        {MediaSize::A2, "A2P"},
        {MediaSize::A3, "A3P"},
        {MediaSize::A4, "A4P"},
        {MediaSize::LETTER, "LETTERP"},
        {MediaSize::LEGAL, "LEGALP"},
        {MediaSize::TABLOID, "11x17P"}
    };

    // Mapeamento de resolução HPGL
    std::map<int, int> resolution_map_ = {
        {300, 300},
        {600, 600},
        {1200, 1200}
    };

public:
    HPGLGenerator(bool use_hpgl2 = true);

    std::vector<uint8_t> generate_header(
        const PlotterCapabilities& caps,
        MediaSize media_size,
        ColorMode color_mode,
        int dpi) override;

    std::vector<uint8_t> generate_page(
        const std::vector<uint8_t>& raster_data,
        int width,
        int height,
        int dpi) override;

    std::vector<uint8_t> generate_footer() override;

    bool validate_media_size(MediaSize size) const override;
    bool validate_resolution(int dpi) const override;
    bool validate_color_mode(ColorMode mode) const override;

    std::string get_protocol_name() const override;
    PlotterCapabilities get_capabilities() const override;

    std::vector<uint8_t> optimize_for_vendor(
        const std::vector<uint8_t>& data) override;

    bool needs_preprocessing() const override;
};

}  // namespace protocols
}  // namespace all_press


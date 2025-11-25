#pragma once

#include "plotter_protocol_base.h"
#include <sstream>

namespace all_press {
namespace protocols {

class PostScriptGenerator : public PlotterProtocolBase {
private:
    PlotterCapabilities capabilities_;
    PlotterVendor target_vendor_;
    
    // Mapeamento de tamanho em PostScript (em pontos)
    std::map<MediaSize, std::pair<float, float>> media_dimensions_ = {
        {MediaSize::A0, {2384, 3370}},
        {MediaSize::A1, {1684, 2384}},
        {MediaSize::A2, {1191, 1684}},
        {MediaSize::A3, {842, 1191}},
        {MediaSize::A4, {595, 842}},
        {MediaSize::LETTER, {612, 792}},
        {MediaSize::LEGAL, {612, 1008}},
        {MediaSize::TABLOID, {792, 1224}}
    };

public:
    PostScriptGenerator(PlotterVendor vendor);

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


#pragma once

#include "plotter_protocol_base.h"
#include <map>
#include <vector>
#include <algorithm>

namespace all_press {
namespace protocols {

struct CompatibilityInfo {
    PlotterVendor vendor;
    std::string model;
    std::vector<std::string> supported_protocols;  // "HPGL", "PostScript", "ESC/P"
    std::string primary_protocol;
    std::vector<std::string> fallback_protocols;
    bool requires_preprocessing;
    std::map<std::string, std::string> quirks;  // Known issues and workarounds
};

class CompatibilityMatrix {
private:
    static const std::map<std::string, CompatibilityInfo> COMPATIBILITY_DB;

    static std::string get_key(PlotterVendor vendor, const std::string& model);

public:
    static bool is_compatible(
        PlotterVendor vendor,
        const std::string& model,
        const std::string& protocol);

    static std::string get_recommended_protocol(
        PlotterVendor vendor,
        const std::string& model);

    static std::vector<std::string> get_fallback_protocols(
        PlotterVendor vendor,
        const std::string& model);

    static std::map<std::string, std::string> get_quirks(
        PlotterVendor vendor,
        const std::string& model);

    static std::vector<CompatibilityInfo> get_all_plotters();
};

}  // namespace protocols
}  // namespace all_press


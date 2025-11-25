#pragma once

#include "plotter_protocol_base.h"
#include "hpgl_generator.h"
#include "postscript_generator.h"
#include "compatibility_matrix.h"
#include <memory>
#include <stdexcept>
#include <algorithm>

namespace all_press {
namespace protocols {

class PlotterProtocolFactory {
public:
    static std::unique_ptr<PlotterProtocolBase> create_protocol(
        const std::string& protocol_name,
        PlotterVendor vendor);

    static std::unique_ptr<PlotterProtocolBase> create_for_printer(
        PlotterVendor vendor,
        const std::string& model);

    static std::vector<std::string> get_available_protocols(
        PlotterVendor vendor,
        const std::string& model);
};

}  // namespace protocols
}  // namespace all_press


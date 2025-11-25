#include "protocols/compatibility_matrix.h"

namespace all_press {
namespace protocols {

std::string CompatibilityMatrix::get_key(PlotterVendor vendor, const std::string& model) {
    std::string prefix;
    if (vendor == PlotterVendor::HP) {
        prefix = "HP_";
    } else if (vendor == PlotterVendor::CANON) {
        prefix = "CANON_";
    } else if (vendor == PlotterVendor::EPSON) {
        prefix = "EPSON_";
    }
    return prefix + model;
}

// Base de dados de compatibilidade
const std::map<std::string, CompatibilityInfo> CompatibilityMatrix::COMPATIBILITY_DB = {
    // HP PLOTTERS
    {
        "HP_DesignJet_T1200",
        CompatibilityInfo{
            .vendor = PlotterVendor::HP,
            .model = "DesignJet T1200",
            .supported_protocols = {"HPGL2", "PostScript", "PDF"},
            .primary_protocol = "HPGL2",
            .fallback_protocols = {"PostScript", "PDF"},
            .requires_preprocessing = true,
            .quirks = {
                {"paper_feed_delay", "500ms"},
                {"pen_warmup", "true"}
            }
        }
    },
    {
        "HP_DesignJet_T2300",
        CompatibilityInfo{
            .vendor = PlotterVendor::HP,
            .model = "DesignJet T2300",
            .supported_protocols = {"HPGL2", "PostScript", "PDF"},
            .primary_protocol = "HPGL2",
            .fallback_protocols = {"PostScript", "PDF"},
            .requires_preprocessing = true,
            .quirks = {
                {"paper_feed_delay", "300ms"},
                {"color_calibration", "required"}
            }
        }
    },
    {
        "HP_DesignJet_T3500",
        CompatibilityInfo{
            .vendor = PlotterVendor::HP,
            .model = "DesignJet T3500",
            .supported_protocols = {"HPGL2", "PostScript", "PDF"},
            .primary_protocol = "HPGL2",
            .fallback_protocols = {"PostScript", "PDF"},
            .requires_preprocessing = true,
            .quirks = {
                {"paper_feed_delay", "200ms"},
                {"high_speed_mode", "true"}
            }
        }
    },
    
    // CANON PLOTTERS
    {
        "CANON_imagePROGRAF_TX_3000",
        CompatibilityInfo{
            .vendor = PlotterVendor::CANON,
            .model = "imagePROGRAF TX-3000",
            .supported_protocols = {"PostScript", "PDF", "HPGL2"},
            .primary_protocol = "PostScript",
            .fallback_protocols = {"PDF", "HPGL2"},
            .requires_preprocessing = false,
            .quirks = {
                {"icc_profile_required", "true"},
                {"ultrachrome_ink_support", "true"}
            }
        }
    },
    {
        "CANON_imagePROGRAF_TX_4000",
        CompatibilityInfo{
            .vendor = PlotterVendor::CANON,
            .model = "imagePROGRAF TX-4000",
            .supported_protocols = {"PostScript", "PDF", "HPGL2"},
            .primary_protocol = "PostScript",
            .fallback_protocols = {"PDF", "HPGL2"},
            .requires_preprocessing = false,
            .quirks = {
                {"icc_profile_required", "true"},
                {"lucia_pro_ink", "true"}
            }
        }
    },
    {
        "CANON_imagePROGRAF_PRO_6000",
        CompatibilityInfo{
            .vendor = PlotterVendor::CANON,
            .model = "imagePROGRAF PRO-6000",
            .supported_protocols = {"PostScript", "PDF"},
            .primary_protocol = "PostScript",
            .fallback_protocols = {"PDF"},
            .requires_preprocessing = false,
            .quirks = {
                {"12_color_ink", "true"},
                {"professional_grade", "true"}
            }
        }
    },
    
    // EPSON PLOTTERS
    {
        "EPSON_SureColor_T5200",
        CompatibilityInfo{
            .vendor = PlotterVendor::EPSON,
            .model = "SureColor T5200",
            .supported_protocols = {"PostScript", "ESC/P", "PDF"},
            .primary_protocol = "PostScript",
            .fallback_protocols = {"ESC/P", "PDF"},
            .requires_preprocessing = false,
            .quirks = {
                {"max_roll_width", "1118mm"},
                {"surecolor_mode", "true"}
            }
        }
    },
    {
        "EPSON_SureColor_T7200",
        CompatibilityInfo{
            .vendor = PlotterVendor::EPSON,
            .model = "SureColor T7200",
            .supported_protocols = {"PostScript", "ESC/P", "PDF"},
            .primary_protocol = "PostScript",
            .fallback_protocols = {"ESC/P", "PDF"},
            .requires_preprocessing = false,
            .quirks = {
                {"max_roll_width", "1118mm"},
                {"ultrachrome_xd2", "true"}
            }
        }
    },
    {
        "EPSON_SureColor_T7700",
        CompatibilityInfo{
            .vendor = PlotterVendor::EPSON,
            .model = "SureColor T7700",
            .supported_protocols = {"PostScript", "ESC/P", "PDF"},
            .primary_protocol = "PostScript",
            .fallback_protocols = {"ESC/P", "PDF"},
            .requires_preprocessing = false,
            .quirks = {
                {"max_roll_width", "1118mm"},
                {"dual_roll_support", "true"}
            }
        }
    }
};

bool CompatibilityMatrix::is_compatible(
    PlotterVendor vendor,
    const std::string& model,
    const std::string& protocol) {
    
    std::string key = get_key(vendor, model);
    
    auto it = COMPATIBILITY_DB.find(key);
    if (it == COMPATIBILITY_DB.end()) {
        return false;
    }
    
    const auto& info = it->second;
    return std::find(info.supported_protocols.begin(),
                    info.supported_protocols.end(),
                    protocol) != info.supported_protocols.end();
}

std::string CompatibilityMatrix::get_recommended_protocol(
    PlotterVendor vendor,
    const std::string& model) {
    
    std::string key = get_key(vendor, model);
    auto it = COMPATIBILITY_DB.find(key);
    
    if (it == COMPATIBILITY_DB.end()) {
        // Default fallback
        if (vendor == PlotterVendor::HP) {
            return "HPGL2";
        } else {
            return "PostScript";
        }
    }
    
    return it->second.primary_protocol;
}

std::vector<std::string> CompatibilityMatrix::get_fallback_protocols(
    PlotterVendor vendor,
    const std::string& model) {
    
    std::string key = get_key(vendor, model);
    auto it = COMPATIBILITY_DB.find(key);
    
    if (it == COMPATIBILITY_DB.end()) {
        return {"PostScript", "HPGL2", "ESC/P"};
    }
    
    return it->second.fallback_protocols;
}

std::map<std::string, std::string> CompatibilityMatrix::get_quirks(
    PlotterVendor vendor,
    const std::string& model) {
    
    std::string key = get_key(vendor, model);
    auto it = COMPATIBILITY_DB.find(key);
    
    if (it == COMPATIBILITY_DB.end()) {
        return {};
    }
    
    return it->second.quirks;
}

std::vector<CompatibilityInfo> CompatibilityMatrix::get_all_plotters() {
    std::vector<CompatibilityInfo> plotters;
    for (const auto& [key, info] : COMPATIBILITY_DB) {
        plotters.push_back(info);
    }
    return plotters;
}

}  // namespace protocols
}  // namespace all_press


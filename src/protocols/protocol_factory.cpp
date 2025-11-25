#include "protocols/protocol_factory.h"

namespace all_press {
namespace protocols {

std::unique_ptr<PlotterProtocolBase> PlotterProtocolFactory::create_protocol(
    const std::string& protocol_name,
    PlotterVendor vendor) {
    
    if (protocol_name == "HPGL" || protocol_name == "HPGL2") {
        return std::make_unique<HPGLGenerator>(protocol_name == "HPGL2");
    }
    else if (protocol_name == "PostScript") {
        return std::make_unique<PostScriptGenerator>(vendor);
    }
    else if (protocol_name == "ESC/P") {
        // TODO: Implementar ESCPGenerator
        throw std::runtime_error("ESC/P not yet implemented");
    }
    else {
        throw std::invalid_argument("Unknown protocol: " + protocol_name);
    }
}

std::unique_ptr<PlotterProtocolBase> PlotterProtocolFactory::create_for_printer(
    PlotterVendor vendor,
    const std::string& model) {
    
    // Obter protocolo recomendado da matriz de compatibilidade
    std::string protocol = CompatibilityMatrix::get_recommended_protocol(
        vendor, model);
    
    return create_protocol(protocol, vendor);
}

std::vector<std::string> PlotterProtocolFactory::get_available_protocols(
    PlotterVendor vendor,
    const std::string& model) {
    
    auto protocols = CompatibilityMatrix::get_fallback_protocols(
        vendor, model);
    
    // Adicionar protocolo primário no início
    std::string primary = CompatibilityMatrix::get_recommended_protocol(
        vendor, model);
    
    protocols.erase(
        std::remove(protocols.begin(), protocols.end(), primary),
        protocols.end());
    
    protocols.insert(protocols.begin(), primary);
    return protocols;
}

}  // namespace protocols
}  // namespace all_press


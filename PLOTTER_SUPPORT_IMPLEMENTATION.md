# All Press C++ - Suporte a Plotters HP, Canon e Epson

## 1. ESTENSÃO DA ARQUITETURA

### 1.1 Nova Camada: Protocol Abstraction Layer

```
┌─────────────────────────────────────────────────────────────┐
│                    Camada de API                            │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   Camada de Negócio                         │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│            🆕 CAMADA DE PROTOCOLO DE PLOTTER               │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   HPGL/2    │  │  PostScript  │  │    ESC/P     │      │
│  │  Generator  │  │   Generator  │  │   Generator  │      │
│  └─────────────┘  └──────────────┘  └──────────────┘      │
│  ┌──────────────────────────────────────────────────┐      │
│  │  PlotterProtocolFactory & Compatibility Matrix  │      │
│  └──────────────────────────────────────────────────┘      │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                 Camada de Conversão                         │
└─────────────────────────────────────────────────────────────┘
```

## 2. NOVOS COMPONENTES

### 2.1 PlotterProtocolBase (Interface Abstrata)

```cpp
// include/protocols/plotter_protocol_base.h

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
```

### 2.2 HPGL/HPGL2 Generator

```cpp
// src/protocols/hpgl_generator.cpp

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
    HPGLGenerator(bool use_hpgl2 = true) : use_hpgl2_(use_hpgl2) {
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
        capabilities_.max_paper_width_mm = 1118;  // 44 polegadas
        capabilities_.max_paper_height_mm = 1600; // ou comprimento de roll
    }

    std::vector<uint8_t> generate_header(
        const PlotterCapabilities& caps,
        MediaSize media_size,
        ColorMode color_mode,
        int dpi) override {
        
        std::string header;
        
        // Inicialização do plotter
        header += "\x1B.@";  // Reset
        header += "ES";      // Enter Setup Mode (HPGL2)
        
        if (use_hpgl2_) {
            header += "\x1B%0B";  // Enter HPGL2 mode
        }
        
        // Page Setup
        header += "PU0,0;";  // Pen Up at origin
        header += "PA0,0;";  // Plot Absolute at origin
        
        // Media Configuration
        std::string media_cmd = "PM" + media_size_map_[media_size] + ";";
        header += media_cmd;
        
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

    std::vector<uint8_t> generate_page(
        const std::vector<uint8_t>& raster_data,
        int width,
        int height,
        int dpi) override {
        
        std::vector<uint8_t> result;
        
        // Converter raster para comandos HPGL
        // Aqui você processaria os dados raster e geraria
        // comandos PA (Plot Absolute) e PU/PD (Pen Up/Down)
        
        // Para simplificar, retornamos os dados brutos com header
        result.insert(result.end(), raster_data.begin(), raster_data.end());
        
        return result;
    }

    std::vector<uint8_t> generate_footer() override {
        std::string footer;
        
        footer += "PU;";           // Pen Up
        footer += "\x1BE";         // Exit plot mode
        
        if (use_hpgl2_) {
            footer += "\x1B%0A";   // Exit HPGL2 mode
        }
        
        footer += "\x1B.@";        // Reset
        
        std::vector<uint8_t> result(footer.begin(), footer.end());
        return result;
    }

    bool validate_media_size(MediaSize size) const override {
        return media_size_map_.count(size) > 0;
    }

    bool validate_resolution(int dpi) const override {
        return resolution_map_.count(dpi) > 0;
    }

    bool validate_color_mode(ColorMode mode) const override {
        return (mode == ColorMode::MONOCHROME || 
                (use_hpgl2_ && mode == ColorMode::COLOR));
    }

    std::string get_protocol_name() const override {
        return use_hpgl2_ ? "HPGL2" : "HPGL";
    }

    PlotterCapabilities get_capabilities() const override {
        return capabilities_;
    }

    std::vector<uint8_t> optimize_for_vendor(
        const std::vector<uint8_t>& data) override {
        // Otimizações específicas de HP
        // Ex: compressão de dados, buffering
        return data;
    }

    bool needs_preprocessing() const override {
        return true;  // HPGL requer conversão de raster
    }
};

}  // namespace protocols
}  // namespace all_press
```

### 2.3 PostScript Generator (Canon/Epson)

```cpp
// src/protocols/postscript_generator.cpp

#pragma once

#include "plotter_protocol_base.h"
#include <sstream>

namespace all_press {
namespace protocols {

class PostScriptGenerator : public PlotterProtocolBase {
private:
    PlotterCapabilities capabilities_;
    PlotterVendor target_vendor_;
    
    // Mapeamento de tamanho em PostScript
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
    PostScriptGenerator(PlotterVendor vendor) 
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
        capabilities_.max_paper_width_mm = 1118;
        capabilities_.max_paper_height_mm = 1600;
    }

    std::vector<uint8_t> generate_header(
        const PlotterCapabilities& caps,
        MediaSize media_size,
        ColorMode color_mode,
        int dpi) override {
        
        std::stringstream ps;
        
        // PostScript Header
        ps << "%!PS-Adobe-3.0\n";
        ps << "%%Creator: All Press C++\n";
        ps << "%%CreationDate: now\n";
        
        // Media Configuration
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

    std::vector<uint8_t> generate_page(
        const std::vector<uint8_t>& raster_data,
        int width,
        int height,
        int dpi) override {
        
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

    std::vector<uint8_t> generate_footer() override {
        std::string footer = "grestore\nshowpage\n%%EOF\n";
        std::vector<uint8_t> result(footer.begin(), footer.end());
        return result;
    }

    bool validate_media_size(MediaSize size) const override {
        return media_dimensions_.count(size) > 0;
    }

    bool validate_resolution(int dpi) const override {
        return (dpi == 300 || dpi == 600 || dpi == 720 || dpi == 1200);
    }

    bool validate_color_mode(ColorMode mode) const override {
        return (mode == ColorMode::MONOCHROME || mode == ColorMode::COLOR);
    }

    std::string get_protocol_name() const override {
        return "PostScript";
    }

    PlotterCapabilities get_capabilities() const override {
        return capabilities_;
    }

    std::vector<uint8_t> optimize_for_vendor(
        const std::vector<uint8_t>& data) override {
        
        if (target_vendor_ == PlotterVendor::CANON) {
            // Otimizações para Canon
            // Ex: Adobe CMYK rendering
        } else if (target_vendor_ == PlotterVendor::EPSON) {
            // Otimizações para Epson
            // Ex: UltraChrome ink optimization
        }
        
        return data;
    }

    bool needs_preprocessing() const override {
        return false;  // PostScript é mais universal
    }
};

}  // namespace protocols
}  // namespace all_press
```

### 2.4 Compatibility Matrix

```cpp
// src/protocols/compatibility_matrix.cpp

#pragma once

#include "plotter_protocol_base.h"
#include <map>
#include <vector>

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

public:
    static bool is_compatible(
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

    static std::string get_recommended_protocol(
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

    static std::vector<std::string> get_fallback_protocols(
        PlotterVendor vendor,
        const std::string& model) {
        
        std::string key = get_key(vendor, model);
        auto it = COMPATIBILITY_DB.find(key);
        
        if (it == COMPATIBILITY_DB.end()) {
            return {"PostScript", "HPGL2", "ESC/P"};
        }
        
        return it->second.fallback_protocols;
    }

    static std::map<std::string, std::string> get_quirks(
        PlotterVendor vendor,
        const std::string& model) {
        
        std::string key = get_key(vendor, model);
        auto it = COMPATIBILITY_DB.find(key);
        
        if (it == COMPATIBILITY_DB.end()) {
            return {};
        }
        
        return it->second.quirks;
    }

private:
    static std::string get_key(PlotterVendor vendor, const std::string& model) {
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
};

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
    }
};

}  // namespace protocols
}  // namespace all_press
```

### 2.5 PlotterProtocolFactory

```cpp
// src/protocols/protocol_factory.cpp

#pragma once

#include "plotter_protocol_base.h"
#include "hpgl_generator.h"
#include "postscript_generator.h"
#include <memory>
#include <stdexcept>

namespace all_press {
namespace protocols {

class PlotterProtocolFactory {
public:
    static std::unique_ptr<PlotterProtocolBase> create_protocol(
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

    static std::unique_ptr<PlotterProtocolBase> create_for_printer(
        PlotterVendor vendor,
        const std::string& model) {
        
        // Obter protocolo recomendado da matriz de compatibilidade
        std::string protocol = CompatibilityMatrix::get_recommended_protocol(
            vendor, model);
        
        return create_protocol(protocol, vendor);
    }

    static std::vector<std::string> get_available_protocols(
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
};

}  // namespace protocols
}  // namespace all_press
```

## 3. INTEGRAÇÃO COM PRINTERMANAGER

### 3.1 Extensão da classe PrinterManager

```cpp
// Modified section in include/core/printer_manager.h

class PrinterManager {
    // ... existing code ...
    
    // 🆕 Novas funções para suporte a plotters
    struct PrinterAdvancedInfo {
        PrinterInfo base_info;
        PlotterVendor vendor;
        std::vector<std::string> supported_protocols;
        std::string recommended_protocol;
        PlotterCapabilities capabilities;
        std::map<std::string, std::string> manufacturer_specs;
    };
    
    // Descobrir impressoras com informações avançadas
    std::future<std::vector<PrinterAdvancedInfo>> 
        discover_plotters_advanced_async();
    
    // Validar documento contra capacidades do plotter
    bool validate_document_for_plotter(
        const std::string& printer_uri,
        const std::string& file_path,
        const PrintOptions& options);
    
    // Obter informações de compatibilidade
    PrinterAdvancedInfo get_plotter_info(
        const std::string& printer_uri);
    
    // Selecionar protocolo automaticamente
    std::string select_best_protocol(
        const std::string& printer_uri,
        const PrintOptions& options);
    
private:
    CompatibilityMatrix compatibility_db_;
    std::map<std::string, PrinterAdvancedInfo> plotter_cache_;
};
```

## 4. EXTENSÃO DO JOBQUEUE

### 4.1 Processamento com Protocol Conversion

```cpp
// Modified section in include/core/job_queue.h

class JobQueue {
    // ... existing code ...
    
    struct ProcessingContext {
        int job_id;
        PrintJob job;
        std::unique_ptr<PlotterProtocolBase> protocol_handler;
        std::string target_protocol;
        PlotterCapabilities target_capabilities;
    };
    
    // Worker melhorado com conversão de protocol
    void process_job_with_protocol(const ProcessingContext& context);
    
    // Pre-flight checks
    bool validate_job_compatibility(const PrintJob& job);
    
private:
    std::map<std::string, std::unique_ptr<PlotterProtocolBase>> 
        protocol_cache_;
};
```

## 5. DADOS DOS FABRICANTES

### 5.1 HP Specification Reference

**Documentação Oficial:**
- HP HPGL2 Command Reference
- HP DesignJet Series Technical Manual
- URL: https://www.hp.com/en/en/support.html

**Especificações:**
```
Plotters: DesignJet T-Series (T1200, T2300, T3500, etc.)
Protocolos: HPGL/HPGL2, PostScript, PDF
Tamanhos: A0 até A5, Roll media
Resoluções: 300, 600, 1200 DPI
Cores: Monochrome, Color (até 12 cores)
```

### 5.2 Canon Specification Reference

**Documentação Oficial:**
- Canon imagePROGRAF Technical Reference
- Canon Print API Documentation
- URL: https://www.canon.com/en/business/

**Especificações:**
```
Plotters: imagePROGRAF TX/TX-PRO Series
Protocolos: PostScript Level 3, PDF, HPGL2
Tamanhos: A0 até A5, Roll media até 1118mm
Resoluções: 300, 600, 720, 1200 DPI
Cores: 5-color LUCIA pigment inks
Perfil ICC: Obrigatório para color management
```

### 5.3 Epson Specification Reference

**Documentação Oficial:**
- Epson SureColor Technical Guide
- Epson UltraChrome INK Reference
- URL: https://epson.com/en/support

**Especificações:**
```
Plotters: SureColor SC-T Series (T5200, T7200, T7700, etc.)
Protocolos: PostScript, ESC/P, PDF
Tamanhos: A0 até A5, Roll media até 1118mm
Resoluções: 300, 600, 720, 1200 DPI
Cores: UltraChrome Pigment Inks
Gerenciamento: ICC Profile support, Color Management
```

## 6. ENDPOINTS REST EXPANDIDOS

```cpp
// Novos endpoints na API

// Descobrir plotters com informações avançadas
GET /api/plotters/discover

// Obter capabilities de um plotter específico
GET /api/plotters/{id}/capabilities

// Listar protocolos suportados
GET /api/plotters/{id}/protocols

// Validar documento contra plotter
POST /api/plotters/{id}/validate
{
    "file_path": "/tmp/document.pdf",
    "media_size": "A3",
    "color_mode": "color",
    "resolution": 600
}

// Enviar com seleção automática de protocolo
POST /api/print/auto
{
    "printer": "hp_designjet_t1200",
    "file_path": "/tmp/document.pdf",
    "options": {
        "auto_select_protocol": true,
        "preferred_protocol": "HPGL2"
    }
}

// Obter matriz de compatibilidade
GET /api/compatibility-matrix

// Consultar quirks/workarounds conhecidos
GET /api/plotters/{vendor}/{model}/quirks
```

## 7. ESTRUTURA DE PASTAS ATUALIZADA

```
all_press_cpp/
├── src/
│   ├── core/               # Engine principal (existente)
│   ├── network/            # Descoberta (existente)
│   ├── conversion/         # Processamento (existente)
│   ├── api/                # REST API (existente)
│   ├── database/           # SQLite (existente)
│   ├── utils/              # Utilitários (existente)
│   └── protocols/          # 🆕 NOVA CAMADA
│       ├── plotter_protocol_base.cpp
│       ├── hpgl_generator.cpp
│       ├── postscript_generator.cpp
│       ├── escp_generator.cpp
│       ├── protocol_factory.cpp
│       ├── compatibility_matrix.cpp
│       └── vendor_specs.cpp
├── include/
│   ├── core/               # Headers existentes
│   ├── network/
│   ├── conversion/
│   ├── api/
│   ├── database/
│   ├── utils/
│   └── protocols/          # 🆕 NOVA CAMADA
│       ├── plotter_protocol_base.h
│       ├── hpgl_generator.h
│       ├── postscript_generator.h
│       ├── escp_generator.h
│       ├── protocol_factory.h
│       └── compatibility_matrix.h
├── config/
│   ├── all_press.conf      # Existente
│   └── plotter_specs.json  # 🆕 Specs dos fabricantes
├── docs/
│   ├── PLOTTER_SUPPORT.md  # 🆕 Este documento
│   ├── MANUFACTURER_SPECS.md # 🆕 Specs detalhadas
│   └── COMPATIBILITY_MATRIX.md # 🆕 Matriz completa
└── tests/
    ├── test_protocols.cpp  # 🆕 Testes de protocols
    ├── test_factory.cpp    # 🆕 Testes de factory
    └── test_compat.cpp     # 🆕 Testes de compatibilidade
```

## 8. CMakeLists.txt - Novos componentes

```cmake
# Adicionar a seção de protocolos

add_library(all_press_protocols
    src/protocols/plotter_protocol_base.cpp
    src/protocols/hpgl_generator.cpp
    src/protocols/postscript_generator.cpp
    src/protocols/escp_generator.cpp
    src/protocols/protocol_factory.cpp
    src/protocols/compatibility_matrix.cpp
    src/protocols/vendor_specs.cpp
)

target_include_directories(all_press_protocols PUBLIC include)
target_link_libraries(all_press_protocols PRIVATE spdlog::spdlog)

# Link ao alvo principal
target_link_libraries(all_press_server PRIVATE all_press_protocols)
```

## 9. ROADMAP DE IMPLEMENTAÇÃO

### Fase 1: Foundation (Semana 1-2)
- [ ] Implementar `PlotterProtocolBase`
- [ ] Criar `HPGLGenerator`
- [ ] Criar `PostScriptGenerator`
- [ ] Implementar `CompatibilityMatrix`
- [ ] Testes unitários básicos

### Fase 2: Integration (Semana 3)
- [ ] Integrar com `PrinterManager`
- [ ] Adicionar `PlotterProtocolFactory`
- [ ] Estender `JobQueue`
- [ ] Testes de integração

### Fase 3: API & Testing (Semana 4)
- [ ] Novos endpoints REST
- [ ] WebSocket events
- [ ] Testes end-to-end
- [ ] Documentação completa

### Fase 4: Advanced (Semana 5+)
- [ ] ESC/P Generator
- [ ] Performance tuning
- [ ] Advanced color management
- [ ] Machine learning para otimização

## 10. CRITÉRIOS DE SUCESSO

- ✅ Suporte completo a HPGL2, PostScript, ESC/P
- ✅ Detecção automática de compatibilidade
- ✅ Fallback automático de protocolo
- ✅ Validação pré-impressão
- ✅ Taxa de sucesso >99% em testes
- ✅ Performance <100ms para decisão de protocolo
- ✅ Documentação 100% coberta
- ✅ Suporte a HP, Canon e Epson principais modelos

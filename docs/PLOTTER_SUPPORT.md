# Suporte a Plotters - All Press C++

## Visão Geral

O All Press C++ agora inclui suporte completo para plotters de grande formato das marcas HP, Canon e Epson. Esta funcionalidade adiciona uma nova camada de protocolo que permite comunicação otimizada com diferentes modelos de plotters usando seus protocolos nativos.

## Protocolos Suportados

### HPGL/HPGL2 (HP Graphics Language)
- **Fabricante**: HP
- **Modelos**: DesignJet T-Series (T1200, T2300, T3500)
- **Melhor para**: Desenhos CAD, diagramas técnicos
- **Pré-processamento**: Necessário

### PostScript
- **Fabricantes**: HP, Canon, Epson
- **Modelos**: Todos os modelos principais
- **Melhor para**: Documentos, gráficos, fotos
- **Pré-processamento**: Não necessário

### ESC/P (Epson Standard Code)
- **Fabricante**: Epson
- **Modelos**: SureColor T-Series
- **Melhor para**: Texto e gráficos simples
- **Pré-processamento**: Não necessário

## Modelos Suportados

### HP
- DesignJet T1200
- DesignJet T2300
- DesignJet T3500

### Canon
- imagePROGRAF TX-3000
- imagePROGRAF TX-4000
- imagePROGRAF PRO-6000

### Epson
- SureColor T5200
- SureColor T7200
- SureColor T7700

## Uso via API REST

### Descobrir Plotters

```bash
GET /api/plotters/discover
```

Resposta:
```json
[
  {
    "name": "HP DesignJet T2300",
    "uri": "ipp://192.168.1.100:631/ipp/print",
    "make_model": "HP DesignJet T2300 ePrinter",
    "vendor": "HP",
    "recommended_protocol": "HPGL2",
    "supported_protocols": ["HPGL2", "PostScript", "PDF"],
    "is_online": true
  }
]
```

### Obter Capacidades de um Plotter

```bash
GET /api/plotters/{id}/capabilities
```

Resposta:
```json
{
  "name": "HP DesignJet T2300",
  "vendor": "HP",
  "model": "DesignJet T2300",
  "supported_sizes": ["A0", "A1", "A2", "A3", "A4"],
  "supported_resolutions": [300, 600, 1200],
  "supported_colors": ["monochrome", "color"],
  "supports_duplex": false,
  "max_paper_width_mm": 1118,
  "max_paper_height_mm": 1600
}
```

### Validar Documento

```bash
POST /api/plotters/{id}/validate
Content-Type: application/json

{
  "file_path": "/tmp/drawing.pdf",
  "media_size": "A1",
  "color_mode": "color",
  "resolution": 600
}
```

Resposta:
```json
{
  "valid": true,
  "printer": "ipp://192.168.1.100:631/ipp/print",
  "file": "/tmp/drawing.pdf",
  "message": "Document is compatible with plotter",
  "recommended_protocol": "HPGL2"
}
```

### Imprimir com Seleção Automática de Protocolo

```bash
POST /api/print/auto
Content-Type: application/json

{
  "printer": "ipp://192.168.1.100:631/ipp/print",
  "file_path": "/tmp/drawing.pdf",
  "options": {
    "media_size": "A1",
    "color_mode": "color",
    "copies": 1,
    "quality": 4,
    "auto_select_protocol": true
  }
}
```

Resposta:
```json
{
  "job_id": 42,
  "printer": "ipp://192.168.1.100:631/ipp/print",
  "file": "/tmp/drawing.pdf",
  "selected_protocol": "HPGL2",
  "status": "queued",
  "message": "Job submitted successfully"
}
```

### Obter Matriz de Compatibilidade

```bash
GET /api/compatibility-matrix
```

### Consultar Quirks Conhecidos

```bash
GET /api/plotters/{vendor}/{model}/quirks
```

Exemplo:
```bash
GET /api/plotters/hp/DesignJet_T2300/quirks
```

Resposta:
```json
{
  "vendor": "hp",
  "model": "DesignJet T2300",
  "quirks": {
    "paper_feed_delay": "300ms",
    "color_calibration": "required"
  }
}
```

## Uso Programático em C++

### Descobrir Plotters

```cpp
#include "core/printer_manager.h"

AllPress::PrinterManager manager;

// Descobrir plotters de forma assíncrona
auto future = manager.discover_plotters_advanced_async();
auto plotters = future.get();

for (const auto& plotter : plotters) {
    std::cout << "Plotter: " << plotter.base_info.name << std::endl;
    std::cout << "Vendor: " << vendor_to_string(plotter.vendor) << std::endl;
    std::cout << "Protocol: " << plotter.recommended_protocol << std::endl;
}
```

### Validar Compatibilidade

```cpp
AllPress::PrintOptions options;
options.media_size = "A1";
options.color_mode = "color";
options.quality = 4;

bool is_valid = manager.validate_document_for_plotter(
    printer_uri,
    file_path,
    options
);

if (is_valid) {
    std::string protocol = manager.select_best_protocol(printer_uri, options);
    std::cout << "Selected protocol: " << protocol << std::endl;
}
```

### Criar Protocolo Handler

```cpp
#include "protocols/protocol_factory.h"

using namespace all_press::protocols;

// Criar protocolo para HP
auto protocol = PlotterProtocolFactory::create_protocol("HPGL2", PlotterVendor::HP);

// Ou criar automaticamente baseado no modelo
auto protocol2 = PlotterProtocolFactory::create_for_printer(
    PlotterVendor::HP,
    "DesignJet T2300"
);

// Gerar comandos
PlotterCapabilities caps = protocol->get_capabilities();
auto header = protocol->generate_header(caps, MediaSize::A1, ColorMode::COLOR, 600);
auto page = protocol->generate_page(raster_data, width, height, 600);
auto footer = protocol->generate_footer();
```

## Configuração

### Arquivo plotter_specs.json

O arquivo `config/plotter_specs.json` contém todas as especificações dos fabricantes. Você pode editá-lo para adicionar novos modelos ou atualizar informações existentes.

### Exemplo de Entrada

```json
{
  "model": "DesignJet T2300",
  "protocols": {
    "primary": "HPGL2",
    "supported": ["HPGL2", "PostScript", "PDF"]
  },
  "media": {
    "sizes": ["A0", "A1", "A2", "A3", "A4"],
    "max_width_mm": 1118
  },
  "quirks": {
    "paper_feed_delay": "300ms"
  }
}
```

## Características Avançadas

### Seleção Automática de Protocolo

O sistema seleciona automaticamente o melhor protocolo baseado em:
- Vendor do plotter
- Modelo específico
- Tipo de documento
- Requisitos de qualidade

### Validação Pré-Impressão

Antes de enviar um trabalho, o sistema valida:
- Tamanho do papel suportado
- Modo de cor compatível
- Resolução adequada
- Capacidades específicas do modelo

### Otimizações por Vendor

Cada protocolo implementa otimizações específicas:
- **HP**: Compressão de dados HPGL
- **Canon**: Perfis ICC para gerenciamento de cor
- **Epson**: Otimizações UltraChrome

### Fallback Automático

Se o protocolo primário falhar, o sistema automaticamente tenta protocolos alternativos na ordem de prioridade.

## Troubleshooting

### Plotter não detectado

1. Verifique se o plotter está ligado e na rede
2. Confirme que o CUPS está instalado (macOS/Linux)
3. Execute: `lpstat -p -d` para ver impressoras disponíveis

### Erro de protocolo

1. Verifique o log em `all_press.log`
2. Tente um protocolo alternativo
3. Consulte quirks conhecidos para o modelo

### Qualidade de impressão

1. Verifique se o protocolo recomendado está sendo usado
2. Para Canon, certifique-se de que perfis ICC estão instalados
3. Calibre o plotter se necessário

## Performance

- Descoberta de plotters: < 2s
- Seleção de protocolo: < 100ms
- Validação pré-impressão: < 50ms
- Conversão de protocolo: depende do tamanho do arquivo

## Limitações Atuais

- ESC/P ainda não implementado (planejado para versão futura)
- Conversão de raster simplificada (melhorias planejadas)
- Suporte limitado a modelos específicos (expansível)

## Roadmap

- [ ] Implementar ESC/P Generator
- [ ] Melhorar conversão de raster
- [ ] Adicionar mais modelos de plotters
- [ ] Suporte a perfis ICC personalizados
- [ ] Machine learning para otimização automática

## Referências

- [HP HPGL/2 Reference Guide](https://www.hp.com/hpgl)
- [Canon imagePROGRAF Technical Guide](https://www.canon.com/imageprograf)
- [Epson SureColor Documentation](https://epson.com/surecolor)


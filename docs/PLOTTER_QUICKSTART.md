# Guia Rápido - Suporte a Plotters

Este guia irá ajudá-lo a começar a usar o suporte a plotters do All Press C++ em 5 minutos.

## 1. Verificar Instalação

Certifique-se de que o All Press C++ está compilado com suporte a plotters:

```bash
cd /path/to/All_press_c
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

## 2. Iniciar o Servidor

```bash
./all_press_server
```

O servidor iniciará na porta 8080 (padrão).

## 3. Descobrir Plotters

### Via API REST

```bash
curl http://localhost:8080/api/plotters/discover
```

Resposta exemplo:
```json
[
  {
    "name": "HP DesignJet T2300",
    "uri": "ipp://192.168.1.100:631/ipp/print",
    "vendor": "HP",
    "recommended_protocol": "HPGL2",
    "supported_protocols": ["HPGL2", "PostScript", "PDF"],
    "is_online": true
  }
]
```

### Via C++

```cpp
#include "core/printer_manager.h"

AllPress::PrinterManager manager;
auto future = manager.discover_plotters_advanced_async();
auto plotters = future.get();

for (const auto& plotter : plotters) {
    std::cout << plotter.base_info.name << std::endl;
}
```

## 4. Verificar Capacidades

```bash
curl http://localhost:8080/api/plotters/ipp%3A%2F%2F192.168.1.100%3A631%2Fipp%2Fprint/capabilities
```

## 5. Validar um Documento

```bash
curl -X POST http://localhost:8080/api/plotters/ipp%3A%2F%2F192.168.1.100%3A631%2Fipp%2Fprint/validate \
  -H "Content-Type: application/json" \
  -d '{
    "file_path": "/tmp/drawing.pdf",
    "media_size": "A1",
    "color_mode": "color",
    "resolution": 600
  }'
```

## 6. Imprimir

### Com Seleção Automática de Protocolo

```bash
curl -X POST http://localhost:8080/api/print/auto \
  -H "Content-Type: application/json" \
  -d '{
    "printer": "ipp://192.168.1.100:631/ipp/print",
    "file_path": "/tmp/drawing.pdf",
    "options": {
      "media_size": "A1",
      "color_mode": "color",
      "copies": 1,
      "quality": 4
    }
  }'
```

Resposta:
```json
{
  "job_id": 42,
  "selected_protocol": "HPGL2",
  "status": "queued"
}
```

### Com Protocolo Específico (via C++)

```cpp
#include "core/printer_manager.h"
#include "core/job_queue.h"

AllPress::PrinterManager manager;
AllPress::JobQueue queue(4);
queue.set_printer_manager(&manager);
queue.start();

// Criar job
AllPress::PrintJob job;
job.printer_name = "ipp://192.168.1.100:631/ipp/print";
job.file_path = "/tmp/drawing.pdf";
job.options.media_size = "A1";
job.options.color_mode = "color";
job.options.quality = 4;

// Adicionar à fila
int job_id = queue.add_job(job);
std::cout << "Job ID: " << job_id << std::endl;
```

## 7. Consultar Matriz de Compatibilidade

```bash
curl http://localhost:8080/api/compatibility-matrix
```

## 8. Verificar Quirks de um Modelo

```bash
curl http://localhost:8080/api/plotters/hp/DesignJet_T2300/quirks
```

## Exemplo Completo em Python

```python
import requests
import json

BASE_URL = "http://localhost:8080"

# 1. Descobrir plotters
response = requests.get(f"{BASE_URL}/api/plotters/discover")
plotters = response.json()
print(f"Found {len(plotters)} plotters")

if len(plotters) > 0:
    plotter = plotters[0]
    printer_uri = plotter["uri"]
    
    # 2. Ver capacidades
    response = requests.get(
        f"{BASE_URL}/api/plotters/{requests.utils.quote(printer_uri, safe='')}/capabilities"
    )
    capabilities = response.json()
    print(f"Capabilities: {json.dumps(capabilities, indent=2)}")
    
    # 3. Validar documento
    validate_data = {
        "file_path": "/tmp/drawing.pdf",
        "media_size": "A1",
        "color_mode": "color",
        "resolution": 600
    }
    response = requests.post(
        f"{BASE_URL}/api/plotters/{requests.utils.quote(printer_uri, safe='')}/validate",
        json=validate_data
    )
    validation = response.json()
    print(f"Valid: {validation['valid']}")
    
    # 4. Imprimir
    if validation["valid"]:
        print_data = {
            "printer": printer_uri,
            "file_path": "/tmp/drawing.pdf",
            "options": {
                "media_size": "A1",
                "color_mode": "color",
                "copies": 1,
                "quality": 4
            }
        }
        response = requests.post(f"{BASE_URL}/api/print/auto", json=print_data)
        result = response.json()
        print(f"Job ID: {result['job_id']}")
        print(f"Protocol: {result['selected_protocol']}")
```

## Exemplo Completo em Node.js

```javascript
const axios = require('axios');

const BASE_URL = 'http://localhost:8080';

async function main() {
    try {
        // 1. Descobrir plotters
        const plottersRes = await axios.get(`${BASE_URL}/api/plotters/discover`);
        const plotters = plottersRes.data;
        console.log(`Found ${plotters.length} plotters`);
        
        if (plotters.length === 0) return;
        
        const plotter = plotters[0];
        const printerUri = encodeURIComponent(plotter.uri);
        
        // 2. Ver capacidades
        const capRes = await axios.get(
            `${BASE_URL}/api/plotters/${printerUri}/capabilities`
        );
        console.log('Capabilities:', JSON.stringify(capRes.data, null, 2));
        
        // 3. Validar documento
        const validateRes = await axios.post(
            `${BASE_URL}/api/plotters/${printerUri}/validate`,
            {
                file_path: '/tmp/drawing.pdf',
                media_size: 'A1',
                color_mode: 'color',
                resolution: 600
            }
        );
        console.log('Valid:', validateRes.data.valid);
        
        // 4. Imprimir
        if (validateRes.data.valid) {
            const printRes = await axios.post(`${BASE_URL}/api/print/auto`, {
                printer: plotter.uri,
                file_path: '/tmp/drawing.pdf',
                options: {
                    media_size: 'A1',
                    color_mode: 'color',
                    copies: 1,
                    quality: 4
                }
            });
            console.log('Job ID:', printRes.data.job_id);
            console.log('Protocol:', printRes.data.selected_protocol);
        }
    } catch (error) {
        console.error('Error:', error.message);
    }
}

main();
```

## Próximos Passos

- Leia a [documentação completa](PLOTTER_SUPPORT.md)
- Veja as [especificações dos fabricantes](MANUFACTURER_SPECS.md)
- Explore a [matriz de compatibilidade](http://localhost:8080/api/compatibility-matrix)
- Consulte o [documento de implementação](../PLOTTER_SUPPORT_IMPLEMENTATION.md)

## Troubleshooting

### Plotter não aparece na lista
- Verifique se está ligado e na rede
- Execute `lpstat -p -d` para ver se o CUPS detecta
- Reinicie o serviço CUPS: `sudo systemctl restart cups`

### Erro de protocolo
- Verifique o log: `tail -f all_press.log`
- Tente protocolo alternativo
- Consulte quirks: `/api/plotters/{vendor}/{model}/quirks`

### Qualidade ruim
- Use protocolo recomendado
- Aumente a qualidade (quality: 5)
- Verifique calibração do plotter

## Suporte

- Issues: GitHub Issues
- Documentação: `/docs`
- Logs: `all_press.log`


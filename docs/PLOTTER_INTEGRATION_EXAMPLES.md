# Exemplos de Integração - Suporte a Plotters

Este documento fornece exemplos práticos de integração do suporte a plotters em diferentes linguagens e cenários.

## Índice
- [C++ Nativo](#c-nativo)
- [Python](#python)
- [Node.js / JavaScript](#nodejs--javascript)
- [React / Frontend](#react--frontend)
- [cURL / Shell Scripts](#curl--shell-scripts)
- [Docker](#docker)

---

## C++ Nativo

### Exemplo 1: Descobrir e Listar Plotters

```cpp
#include "core/printer_manager.h"
#include <iostream>

int main() {
    AllPress::PrinterManager manager;
    
    // Descobrir plotters de forma assíncrona
    auto future = manager.discover_plotters_advanced_async();
    std::cout << "Discovering plotters..." << std::endl;
    
    auto plotters = future.get();
    
    std::cout << "Found " << plotters.size() << " plotters:\n" << std::endl;
    
    for (const auto& plotter : plotters) {
        std::cout << "Name: " << plotter.base_info.name << std::endl;
        std::cout << "Vendor: " << vendor_to_string(plotter.vendor) << std::endl;
        std::cout << "Model: " << plotter.base_info.make_model << std::endl;
        std::cout << "Protocol: " << plotter.recommended_protocol << std::endl;
        std::cout << "Online: " << (plotter.base_info.is_online ? "Yes" : "No") << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
```

### Exemplo 2: Validar e Imprimir

```cpp
#include "core/printer_manager.h"
#include "core/job_queue.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    AllPress::PrinterManager manager;
    AllPress::JobQueue queue(4);
    queue.set_printer_manager(&manager);
    queue.start();
    
    std::string printer_uri = "ipp://192.168.1.100:631/ipp/print";
    std::string file_path = "/tmp/drawing.pdf";
    
    // Configurar opções
    AllPress::PrintOptions options;
    options.media_size = "A1";
    options.color_mode = "color";
    options.quality = 4;
    options.copies = 1;
    
    // Validar documento
    bool is_valid = manager.validate_document_for_plotter(
        printer_uri, file_path, options);
    
    if (!is_valid) {
        std::cerr << "Document validation failed!" << std::endl;
        return 1;
    }
    
    // Selecionar protocolo
    std::string protocol = manager.select_best_protocol(printer_uri, options);
    std::cout << "Selected protocol: " << protocol << std::endl;
    
    // Criar job
    AllPress::PrintJob job;
    job.printer_name = printer_uri;
    job.file_path = file_path;
    job.original_filename = "drawing.pdf";
    job.options = options;
    job.created_at = std::chrono::system_clock::now();
    
    // Adicionar à fila
    int job_id = queue.add_job(job);
    std::cout << "Job submitted with ID: " << job_id << std::endl;
    
    // Monitorar progresso
    queue.set_progress_callback([](int id, float progress) {
        std::cout << "Job " << id << " progress: " 
                  << (int)(progress * 100) << "%" << std::endl;
    });
    
    // Aguardar conclusão
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    queue.stop();
    return 0;
}
```

### Exemplo 3: Usar Protocolo Diretamente

```cpp
#include "protocols/protocol_factory.h"
#include "protocols/plotter_protocol_base.h"
#include <fstream>
#include <iostream>

using namespace all_press::protocols;

int main() {
    // Criar protocolo para HP
    auto protocol = PlotterProtocolFactory::create_protocol(
        "HPGL2", PlotterVendor::HP);
    
    // Obter capacidades
    auto caps = protocol->get_capabilities();
    
    // Configurar trabalho
    MediaSize media = MediaSize::A1;
    ColorMode color = ColorMode::COLOR;
    int dpi = 600;
    
    // Validar
    if (!protocol->validate_media_size(media)) {
        std::cerr << "Media size not supported" << std::endl;
        return 1;
    }
    
    // Gerar comandos
    auto header = protocol->generate_header(caps, media, color, dpi);
    
    // Carregar dados raster (simplificado)
    std::vector<uint8_t> raster_data = {/* ... */};
    int width = 7016;  // A1 @ 600 DPI
    int height = 9933;
    
    auto page = protocol->generate_page(raster_data, width, height, dpi);
    auto footer = protocol->generate_footer();
    
    // Combinar
    std::vector<uint8_t> output;
    output.insert(output.end(), header.begin(), header.end());
    output.insert(output.end(), page.begin(), page.end());
    output.insert(output.end(), footer.begin(), footer.end());
    
    // Otimizar
    output = protocol->optimize_for_vendor(output);
    
    // Salvar
    std::ofstream file("output.hpgl", std::ios::binary);
    file.write(reinterpret_cast<const char*>(output.data()), output.size());
    file.close();
    
    std::cout << "Generated " << output.size() << " bytes of HPGL2 data" << std::endl;
    
    return 0;
}
```

---

## Python

### Exemplo 1: Cliente Python Completo

```python
import requests
import json
import time
from typing import List, Dict, Optional

class AllPressPlotterClient:
    def __init__(self, base_url: str = "http://localhost:8080"):
        self.base_url = base_url
        self.session = requests.Session()
    
    def discover_plotters(self) -> List[Dict]:
        """Descobrir plotters disponíveis"""
        response = self.session.get(f"{self.base_url}/api/plotters/discover")
        response.raise_for_status()
        return response.json()
    
    def get_capabilities(self, printer_uri: str) -> Dict:
        """Obter capacidades de um plotter"""
        encoded_uri = requests.utils.quote(printer_uri, safe='')
        response = self.session.get(
            f"{self.base_url}/api/plotters/{encoded_uri}/capabilities"
        )
        response.raise_for_status()
        return response.json()
    
    def get_protocols(self, printer_uri: str) -> Dict:
        """Obter protocolos suportados"""
        encoded_uri = requests.utils.quote(printer_uri, safe='')
        response = self.session.get(
            f"{self.base_url}/api/plotters/{encoded_uri}/protocols"
        )
        response.raise_for_status()
        return response.json()
    
    def validate_document(
        self,
        printer_uri: str,
        file_path: str,
        media_size: str = "A1",
        color_mode: str = "color",
        resolution: int = 600
    ) -> Dict:
        """Validar documento para impressão"""
        encoded_uri = requests.utils.quote(printer_uri, safe='')
        data = {
            "file_path": file_path,
            "media_size": media_size,
            "color_mode": color_mode,
            "resolution": resolution
        }
        response = self.session.post(
            f"{self.base_url}/api/plotters/{encoded_uri}/validate",
            json=data
        )
        response.raise_for_status()
        return response.json()
    
    def print_auto(
        self,
        printer_uri: str,
        file_path: str,
        media_size: str = "A1",
        color_mode: str = "color",
        copies: int = 1,
        quality: int = 4
    ) -> Dict:
        """Imprimir com seleção automática de protocolo"""
        data = {
            "printer": printer_uri,
            "file_path": file_path,
            "options": {
                "media_size": media_size,
                "color_mode": color_mode,
                "copies": copies,
                "quality": quality
            }
        }
        response = self.session.post(
            f"{self.base_url}/api/print/auto",
            json=data
        )
        response.raise_for_status()
        return response.json()
    
    def get_compatibility_matrix(self) -> List[Dict]:
        """Obter matriz de compatibilidade completa"""
        response = self.session.get(
            f"{self.base_url}/api/compatibility-matrix"
        )
        response.raise_for_status()
        return response.json()
    
    def get_quirks(self, vendor: str, model: str) -> Dict:
        """Obter quirks conhecidos de um modelo"""
        response = self.session.get(
            f"{self.base_url}/api/plotters/{vendor}/{model}/quirks"
        )
        response.raise_for_status()
        return response.json()


# Exemplo de uso
def main():
    client = AllPressPlotterClient()
    
    # 1. Descobrir plotters
    print("🔍 Discovering plotters...")
    plotters = client.discover_plotters()
    print(f"Found {len(plotters)} plotters\n")
    
    for plotter in plotters:
        print(f"📄 {plotter['name']}")
        print(f"   Vendor: {plotter['vendor']}")
        print(f"   Protocol: {plotter['recommended_protocol']}")
        print(f"   Online: {'✓' if plotter['is_online'] else '✗'}")
        print()
    
    if not plotters:
        print("No plotters found!")
        return
    
    # 2. Usar primeiro plotter
    plotter = plotters[0]
    printer_uri = plotter['uri']
    
    # 3. Ver capacidades
    print("📊 Capabilities:")
    caps = client.get_capabilities(printer_uri)
    print(f"   Sizes: {', '.join(caps['supported_sizes'])}")
    print(f"   Resolutions: {caps['supported_resolutions']}")
    print(f"   Colors: {', '.join(caps['supported_colors'])}")
    print()
    
    # 4. Validar documento
    print("✅ Validating document...")
    validation = client.validate_document(
        printer_uri,
        "/tmp/drawing.pdf",
        media_size="A1",
        color_mode="color",
        resolution=600
    )
    print(f"   Valid: {validation['valid']}")
    if validation['valid']:
        print(f"   Protocol: {validation['recommended_protocol']}")
    print()
    
    # 5. Imprimir
    if validation['valid']:
        print("🖨️  Submitting print job...")
        result = client.print_auto(
            printer_uri,
            "/tmp/drawing.pdf",
            media_size="A1",
            color_mode="color",
            quality=4
        )
        print(f"   Job ID: {result['job_id']}")
        print(f"   Protocol: {result['selected_protocol']}")
        print(f"   Status: {result['status']}")

if __name__ == "__main__":
    main()
```

---

## Node.js / JavaScript

### Exemplo 1: Cliente Node.js com Async/Await

```javascript
const axios = require('axios');

class AllPressPlotterClient {
    constructor(baseURL = 'http://localhost:8080') {
        this.client = axios.create({ baseURL });
    }

    async discoverPlotters() {
        const { data } = await this.client.get('/api/plotters/discover');
        return data;
    }

    async getCapabilities(printerUri) {
        const encoded = encodeURIComponent(printerUri);
        const { data } = await this.client.get(`/api/plotters/${encoded}/capabilities`);
        return data;
    }

    async getProtocols(printerUri) {
        const encoded = encodeURIComponent(printerUri);
        const { data } = await this.client.get(`/api/plotters/${encoded}/protocols`);
        return data;
    }

    async validateDocument(printerUri, filePath, options = {}) {
        const encoded = encodeURIComponent(printerUri);
        const { data } = await this.client.post(
            `/api/plotters/${encoded}/validate`,
            {
                file_path: filePath,
                media_size: options.mediaSize || 'A1',
                color_mode: options.colorMode || 'color',
                resolution: options.resolution || 600
            }
        );
        return data;
    }

    async printAuto(printerUri, filePath, options = {}) {
        const { data } = await this.client.post('/api/print/auto', {
            printer: printerUri,
            file_path: filePath,
            options: {
                media_size: options.mediaSize || 'A1',
                color_mode: options.colorMode || 'color',
                copies: options.copies || 1,
                quality: options.quality || 4
            }
        });
        return data;
    }

    async getCompatibilityMatrix() {
        const { data } = await this.client.get('/api/compatibility-matrix');
        return data;
    }

    async getQuirks(vendor, model) {
        const { data } = await this.client.get(`/api/plotters/${vendor}/${model}/quirks`);
        return data;
    }
}

// Exemplo de uso
async function main() {
    const client = new AllPressPlotterClient();

    try {
        // Descobrir plotters
        console.log('🔍 Discovering plotters...');
        const plotters = await client.discoverPlotters();
        console.log(`Found ${plotters.length} plotters\n`);

        plotters.forEach(plotter => {
            console.log(`📄 ${plotter.name}`);
            console.log(`   Vendor: ${plotter.vendor}`);
            console.log(`   Protocol: ${plotter.recommended_protocol}`);
            console.log(`   Online: ${plotter.is_online ? '✓' : '✗'}\n`);
        });

        if (plotters.length === 0) {
            console.log('No plotters found!');
            return;
        }

        const plotter = plotters[0];
        const printerUri = plotter.uri;

        // Ver capacidades
        console.log('📊 Capabilities:');
        const caps = await client.getCapabilities(printerUri);
        console.log(`   Sizes: ${caps.supported_sizes.join(', ')}`);
        console.log(`   Resolutions: ${caps.supported_resolutions.join(', ')}`);
        console.log(`   Colors: ${caps.supported_colors.join(', ')}\n`);

        // Validar documento
        console.log('✅ Validating document...');
        const validation = await client.validateDocument(
            printerUri,
            '/tmp/drawing.pdf',
            { mediaSize: 'A1', colorMode: 'color', resolution: 600 }
        );
        console.log(`   Valid: ${validation.valid}`);
        if (validation.valid) {
            console.log(`   Protocol: ${validation.recommended_protocol}\n`);
        }

        // Imprimir
        if (validation.valid) {
            console.log('🖨️  Submitting print job...');
            const result = await client.printAuto(
                printerUri,
                '/tmp/drawing.pdf',
                { mediaSize: 'A1', colorMode: 'color', quality: 4 }
            );
            console.log(`   Job ID: ${result.job_id}`);
            console.log(`   Protocol: ${result.selected_protocol}`);
            console.log(`   Status: ${result.status}`);
        }

    } catch (error) {
        console.error('Error:', error.message);
        if (error.response) {
            console.error('Response:', error.response.data);
        }
    }
}

main();
```

---

## React / Frontend

### Exemplo: Hook Personalizado React

```typescript
import { useState, useEffect, useCallback } from 'react';
import axios from 'axios';

interface Plotter {
    name: string;
    uri: string;
    vendor: string;
    recommended_protocol: string;
    supported_protocols: string[];
    is_online: boolean;
}

interface Capabilities {
    name: string;
    vendor: string;
    model: string;
    supported_sizes: string[];
    supported_resolutions: number[];
    supported_colors: string[];
    max_paper_width_mm: number;
    max_paper_height_mm: number;
}

const usePlotters = (baseURL: string = 'http://localhost:8080') => {
    const [plotters, setPlotters] = useState<Plotter[]>([]);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState<string | null>(null);

    const discover = useCallback(async () => {
        setLoading(true);
        setError(null);
        try {
            const { data } = await axios.get(`${baseURL}/api/plotters/discover`);
            setPlotters(data);
        } catch (err: any) {
            setError(err.message);
        } finally {
            setLoading(false);
        }
    }, [baseURL]);

    const getCapabilities = useCallback(async (printerUri: string): Promise<Capabilities> => {
        const encoded = encodeURIComponent(printerUri);
        const { data } = await axios.get(`${baseURL}/api/plotters/${encoded}/capabilities`);
        return data;
    }, [baseURL]);

    const printAuto = useCallback(async (
        printerUri: string,
        filePath: string,
        options: {
            mediaSize?: string;
            colorMode?: string;
            copies?: number;
            quality?: number;
        } = {}
    ) => {
        const { data } = await axios.post(`${baseURL}/api/print/auto`, {
            printer: printerUri,
            file_path: filePath,
            options: {
                media_size: options.mediaSize || 'A1',
                color_mode: options.colorMode || 'color',
                copies: options.copies || 1,
                quality: options.quality || 4
            }
        });
        return data;
    }, [baseURL]);

    useEffect(() => {
        discover();
    }, [discover]);

    return { plotters, loading, error, discover, getCapabilities, printAuto };
};

// Componente de exemplo
const PlotterDashboard: React.FC = () => {
    const { plotters, loading, error, getCapabilities, printAuto } = usePlotters();
    const [selectedPlotter, setSelectedPlotter] = useState<Plotter | null>(null);
    const [capabilities, setCapabilities] = useState<Capabilities | null>(null);

    const handleSelectPlotter = async (plotter: Plotter) => {
        setSelectedPlotter(plotter);
        const caps = await getCapabilities(plotter.uri);
        setCapabilities(caps);
    };

    const handlePrint = async () => {
        if (!selectedPlotter) return;
        try {
            const result = await printAuto(selectedPlotter.uri, '/tmp/drawing.pdf', {
                mediaSize: 'A1',
                colorMode: 'color',
                quality: 4
            });
            alert(`Job ${result.job_id} submitted with protocol ${result.selected_protocol}`);
        } catch (err: any) {
            alert(`Error: ${err.message}`);
        }
    };

    if (loading) return <div>Loading plotters...</div>;
    if (error) return <div>Error: {error}</div>;

    return (
        <div>
            <h1>Plotter Dashboard</h1>
            
            <div>
                <h2>Available Plotters</h2>
                {plotters.map(plotter => (
                    <div key={plotter.uri} onClick={() => handleSelectPlotter(plotter)}>
                        <h3>{plotter.name}</h3>
                        <p>Vendor: {plotter.vendor}</p>
                        <p>Status: {plotter.is_online ? '🟢 Online' : '🔴 Offline'}</p>
                    </div>
                ))}
            </div>

            {selectedPlotter && capabilities && (
                <div>
                    <h2>Capabilities</h2>
                    <p>Model: {capabilities.model}</p>
                    <p>Sizes: {capabilities.supported_sizes.join(', ')}</p>
                    <p>Resolutions: {capabilities.supported_resolutions.join(', ')} DPI</p>
                    <button onClick={handlePrint}>Print Test</button>
                </div>
            )}
        </div>
    );
};

export default PlotterDashboard;
```

---

## cURL / Shell Scripts

### Script de Deploy Completo

```bash
#!/bin/bash

# Configuração
BASE_URL="http://localhost:8080"
FILE_PATH="/tmp/drawing.pdf"

# Cores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔍 Discovering plotters...${NC}"
PLOTTERS=$(curl -s "${BASE_URL}/api/plotters/discover")
echo "$PLOTTERS" | jq '.'

# Extrair primeiro plotter
PRINTER_URI=$(echo "$PLOTTERS" | jq -r '.[0].uri')
PRINTER_NAME=$(echo "$PLOTTERS" | jq -r '.[0].name')

if [ "$PRINTER_URI" == "null" ]; then
    echo -e "${RED}❌ No plotters found!${NC}"
    exit 1
fi

echo -e "\n${GREEN}✓ Selected plotter: $PRINTER_NAME${NC}"

# Capacidades
echo -e "\n${BLUE}📊 Getting capabilities...${NC}"
ENCODED_URI=$(printf %s "$PRINTER_URI" | jq -sRr @uri)
curl -s "${BASE_URL}/api/plotters/${ENCODED_URI}/capabilities" | jq '.'

# Validar documento
echo -e "\n${BLUE}✅ Validating document...${NC}"
VALIDATION=$(curl -s -X POST "${BASE_URL}/api/plotters/${ENCODED_URI}/validate" \
    -H "Content-Type: application/json" \
    -d "{
        \"file_path\": \"$FILE_PATH\",
        \"media_size\": \"A1\",
        \"color_mode\": \"color\",
        \"resolution\": 600
    }")

echo "$VALIDATION" | jq '.'

IS_VALID=$(echo "$VALIDATION" | jq -r '.valid')

if [ "$IS_VALID" != "true" ]; then
    echo -e "${RED}❌ Document validation failed!${NC}"
    exit 1
fi

# Imprimir
echo -e "\n${BLUE}🖨️  Submitting print job...${NC}"
RESULT=$(curl -s -X POST "${BASE_URL}/api/print/auto" \
    -H "Content-Type: application/json" \
    -d "{
        \"printer\": \"$PRINTER_URI\",
        \"file_path\": \"$FILE_PATH\",
        \"options\": {
            \"media_size\": \"A1\",
            \"color_mode\": \"color\",
            \"copies\": 1,
            \"quality\": 4
        }
    }")

echo "$RESULT" | jq '.'

JOB_ID=$(echo "$RESULT" | jq -r '.job_id')
PROTOCOL=$(echo "$RESULT" | jq -r '.selected_protocol')

echo -e "\n${GREEN}✓ Job submitted successfully!${NC}"
echo -e "  Job ID: $JOB_ID"
echo -e "  Protocol: $PROTOCOL"
```

---

## Docker

### Dockerfile com Suporte a Plotters

```dockerfile
FROM ubuntu:22.04

# Instalar dependências
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcups2-dev \
    libssl-dev \
    libboost-all-dev \
    nlohmann-json3-dev \
    libspdlog-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copiar código
COPY . .

# Compilar com suporte a plotters
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Copiar configurações
COPY config/plotter_specs.json /etc/all_press/plotter_specs.json

EXPOSE 8080

CMD ["./build/all_press_server"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  all_press:
    build: .
    ports:
      - "8080:8080"
    volumes:
      - ./config:/etc/all_press:ro
      - ./logs:/var/log/all_press
      - /tmp:/tmp
    environment:
      - LOG_LEVEL=info
      - ENABLE_PLOTTERS=true
    restart: unless-stopped

  # Exemplo de serviço que usa a API
  print_frontend:
    image: nginx:alpine
    ports:
      - "3000:80"
    volumes:
      - ./frontend:/usr/share/nginx/html:ro
    depends_on:
      - all_press
```

---

## Conclusão

Estes exemplos demonstram como integrar o suporte a plotters do All Press C++ em diferentes linguagens e ambientes. Todos os exemplos são funcionais e podem ser adaptados às suas necessidades específicas.

Para mais informações, consulte:
- [PLOTTER_SUPPORT.md](PLOTTER_SUPPORT.md)
- [PLOTTER_QUICKSTART.md](PLOTTER_QUICKSTART.md)
- [MANUFACTURER_SPECS.md](MANUFACTURER_SPECS.md)


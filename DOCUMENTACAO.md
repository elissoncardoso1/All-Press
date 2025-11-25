# All Press C++ - Documentação Completa

![Version](https://img.shields.io/badge/version-1.1.0-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17-00599C)
![License](https://img.shields.io/badge/license-MIT-green)

Sistema completo de gerenciamento de impressão de alta performance desenvolvido em C/C++ com APIs nativas. Projetado para gráficas rápidas que necessitam de máximo desempenho, robustez e controle granular sobre operações de impressão.

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Instalação](#instalação)
3. [Arquitetura](#arquitetura)
4. [API REST](#api-rest)
5. [Uso e Exemplos](#uso-e-exemplos)
6. [Suporte a Plotters](#suporte-a-plotters)
7. [Desenvolvimento](#desenvolvimento)
8. [Contribuindo](#contribuindo)
9. [Changelog](#changelog)
10. [Troubleshooting](#troubleshooting)

---

## 🚀 Visão Geral

### Características Principais

- **Motor de impressão nativo C++** com performance 10-50x superior ao Python
- **Descoberta assíncrona de impressoras** com threading paralelo
- **Processamento de arquivos** com streaming para documentos grandes
- **Sistema de filas inteligente** com balanceamento de carga
- **Color management nativo** com perfis ICC
- **API REST C++** de alta performance
- **WebSocket server nativo** para atualizações em tempo real
- **Uso de memória 80-90% menor** que soluções Python
- **Suporte completo a plotters** (HP, Canon, Epson)

### Performance Esperada

| Operação | Python/FastAPI | C++ Nativo | Melhoria |
|----------|---------------|------------|----------|
| Descoberta de rede (254 IPs) | 2-5 seg | 0.3-0.8 seg | **10-15x** |
| Processamento PDF 100MB | 45-60 seg | 8-15 seg | **4-6x** |
| Uso de memória base | 150-300 MB | 15-30 MB | **80-90%** |
| Jobs simultâneos | 10-20/min | 50-100/min | **5x** |

---

## 📦 Instalação

### Requisitos do Sistema

#### macOS
```bash
brew install cmake ninja boost openssl sqlite3 cups
```

#### Ubuntu/Debian
```bash
sudo apt-get install -y \
    build-essential cmake ninja-build \
    libcups2-dev libboost-all-dev libssl-dev \
    libsqlite3-dev libcurl4-openssl-dev
```

### Instalação Rápida

```bash
# Clone o repositório
git clone https://github.com/elissoncardoso1/All-Press.git
cd All-Press

# Instalar e compilar tudo
./scripts/install.sh
```

### Build Manual

```bash
# 1. Instalar dependências com Conan (opcional)
pip install conan
conan profile detect --force
mkdir build && cd build
conan install .. --build=missing

# 2. Configurar com CMake
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# 3. Compilar
ninja

# 4. Executar
./all_press_server
```

### Modo Desenvolvimento

```bash
# Inicia o servidor com hot-reload
./scripts/dev.sh
```

---

## 🏗️ Arquitetura

### Estrutura em Camadas

```
┌─────────────────────────────────────────────────────────────┐
│                    Camada de API                            │
│  ┌──────────────────┐      ┌──────────────────┐            │
│  │   HTTP Server    │      │  WebSocket Server │            │
│  │   (REST API)     │      │  (Real-time)      │            │
│  └──────────────────┘      └──────────────────┘            │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   Camada de Negócio                         │
│  ┌────────────┐  ┌─────────────┐  ┌──────────────┐        │
│  │  Printer   │  │  Job Queue  │  │    Color     │        │
│  │  Manager   │  │             │  │   Manager    │        │
│  └────────────┘  └─────────────┘  └──────────────┘        │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                 Camada de Conversão                         │
│  ┌────────────┐  ┌─────────────┐  ┌──────────────┐        │
│  │    File    │  │    Image    │  │     PDF      │        │
│  │ Processor  │  │  Processor  │  │  Processor   │        │
│  └────────────┘  └─────────────┘  └──────────────┘        │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  Camada de Network                          │
│  ┌────────────┐  ┌─────────────┐  ┌──────────────┐        │
│  │    IPP     │  │   Network   │  │     CUPS     │        │
│  │   Client   │  │   Scanner   │  │    Client    │        │
│  └────────────┘  └─────────────┘  └──────────────┘        │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              Camada de Persistência                         │
│  ┌────────────┐  ┌─────────────┐  ┌──────────────┐        │
│  │   SQLite   │  │    Config   │  │    Logger    │        │
│  │  Manager   │  │   Manager   │  │              │        │
│  └────────────┘  └─────────────┘  └──────────────┘        │
└─────────────────────────────────────────────────────────────┘
```

### Estrutura do Projeto

```
all_press_cpp/
├── src/
│   ├── core/              # Engine principal
│   │   ├── printer_manager.cpp
│   │   ├── job_queue.cpp
│   │   └── color_manager.cpp
│   ├── network/           # Descoberta e comunicação
│   │   ├── cups_client.cpp
│   │   ├── ipp_client.cpp
│   │   └── network_scanner.cpp
│   ├── conversion/        # Processamento de arquivos
│   │   ├── file_processor.cpp
│   │   ├── pdf_processor.cpp
│   │   └── image_processor.cpp
│   ├── api/               # REST API & WebSocket
│   │   ├── rest_server.cpp
│   │   └── websocket_server.cpp
│   ├── protocols/         # Protocolos de plotters
│   │   ├── hpgl_generator.cpp
│   │   ├── postscript_generator.cpp
│   │   └── compatibility_matrix.cpp
│   ├── database/          # Persistência SQLite
│   │   └── sqlite_manager.cpp
│   ├── utils/             # Utilitários
│   │   ├── logger.cpp
│   │   ├── config.cpp
│   │   └── file_utils.cpp
│   └── main.cpp           # Aplicação principal
├── include/               # Headers públicos
├── config/                # Arquivos de configuração
├── tests/                 # Testes unitários
├── frontend/              # Interface web React/TypeScript
└── CMakeLists.txt         # Build system
```

### Componentes Principais

#### PrinterManager
- Descoberta de impressoras (CUPS, IPP, mDNS)
- Monitoramento de status
- Gerenciamento de configurações
- Cache de informações
- Suporte a plotters

#### JobQueue
- Fila FIFO com priorização
- Pool de workers configurável
- Processamento assíncrono
- Controle de concorrência
- Persistência de estado

#### FileProcessor
- Detecção automática de tipo
- Conversão para PDF otimizado
- Suporte a múltiplos formatos
- Otimização de imagens

#### ColorManager
- Conversão RGB ↔ CMYK
- Aplicação de ICC profiles
- Calibração de cores
- Gamma correction

---

## 🔌 API REST

**Base URL**: `http://localhost:8000/api`

**Versão**: 1.1.0

### Printers

#### GET /api/printers
Lista todas as impressoras descobertas.

**Resposta**:
```json
[
  {
    "id": "HP_LaserJet",
    "name": "HP LaserJet",
    "status": "online",
    "type": "CUPS",
    "manufacturer": "HP",
    "model": "LaserJet Pro",
    "location": "Office",
    "uri": "ipp://192.168.1.100:631/ipp/print",
    "capabilities": {
      "supportedFormats": ["pdf", "jpg", "png"],
      "colorSupported": true,
      "duplexSupported": true,
      "maxPaperSize": "A4",
      "resolutions": [300, 600],
      "paperSizes": ["A4", "Letter"]
    },
    "currentJobs": 2,
    "totalJobsProcessed": 150
  }
]
```

#### POST /api/printers/discover
Inicia uma nova descoberta de impressoras na rede.

#### GET /api/printers/{id}
Obtém detalhes de uma impressora específica.

### Jobs

#### GET /api/jobs
Lista todos os jobs de impressão.

**Resposta**:
```json
[
  {
    "id": "job_123",
    "printerId": "HP_LaserJet",
    "fileName": "document.pdf",
    "status": "processing",
    "progress": 45.5,
    "createdAt": "2025-01-15T10:30:00Z",
    "options": {
      "copies": 2,
      "colorMode": "color",
      "duplex": "long-edge"
    }
  }
]
```

#### POST /api/jobs
Cria um novo job de impressão.

**Content-Type**: `multipart/form-data`

**Parâmetros**:
- `file` (file): Arquivo a ser impresso
- `printer_id` (string): ID da impressora
- `options` (string): JSON com opções de impressão

**Exemplo**:
```bash
curl -X POST http://localhost:8000/api/jobs \
  -F "file=@document.pdf" \
  -F "printer_id=HP_LaserJet" \
  -F 'options={"copies":2,"colorMode":"color","duplex":"long-edge"}'
```

#### GET /api/jobs/{id}
Obtém status de um job específico.

#### DELETE /api/jobs/{id}
Cancela um job.

### System

#### GET /api/system/status
Status do sistema.

**Resposta**:
```json
{
  "status": "online",
  "uptime": 3600,
  "version": "1.1.0",
  "cupsConnected": true,
  "databaseConnected": true
}
```

#### GET /api/system/metrics
Métricas de performance do sistema.

#### GET /api/system/stats
Estatísticas do sistema.

#### GET /api/system/settings
Obtém todas as configurações.

#### POST /api/system/settings
Salva configurações.

### Plotters

#### GET /api/plotters/discover
Descobrir plotters avançado.

#### GET /api/plotters/{id}/capabilities
Capacidades do plotter.

#### GET /api/plotters/{id}/protocols
Protocolos suportados.

#### POST /api/plotters/{id}/validate
Validar documento antes de imprimir.

---

## 💻 Uso e Exemplos

### Iniciar o Servidor

```bash
cd build
./all_press_server
```

O servidor iniciará nas seguintes portas:
- **API REST**: http://localhost:8000
- **WebSocket**: ws://localhost:8001

### Exemplos de Uso

#### Listar Impressoras
```bash
curl http://localhost:8000/api/printers
```

#### Descobrir Impressoras na Rede
```bash
curl -X POST http://localhost:8000/api/printers/discover
```

#### Criar Job de Impressão
```bash
curl -X POST http://localhost:8000/api/jobs \
  -F "file=@document.pdf" \
  -F "printer_id=HP_LaserJet" \
  -F 'options={"copies":2,"colorMode":"color"}'
```

#### Status de um Job
```bash
curl http://localhost:8000/api/jobs/123
```

### WebSocket - Atualizações em Tempo Real

```javascript
const ws = new WebSocket('ws://localhost:8001');

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log('Update:', data);
};

// Eventos recebidos:
// - job_created
// - job_progress
// - job_completed
// - printer_discovered
// - printer_status_changed
```

### Exemplos de Integração

#### Node.js
```javascript
const axios = require('axios');
const FormData = require('form-data');
const fs = require('fs');

async function printFile(filePath, printerName) {
  const form = new FormData();
  form.append('file', fs.createReadStream(filePath));
  form.append('printer_id', printerName);
  form.append('options', JSON.stringify({ copies: 2 }));

  const response = await axios.post('http://localhost:8000/api/jobs', form, {
    headers: form.getHeaders()
  });
  
  return response.data;
}
```

#### Python
```python
import requests

def print_file(file_path, printer_name):
    with open(file_path, 'rb') as f:
        files = {'file': f}
        data = {
            'printer_id': printer_name,
            'options': '{"copies": 2}'
        }
        response = requests.post('http://localhost:8000/api/jobs', 
                                files=files, data=data)
        return response.json()
```

---

## 🖨️ Suporte a Plotters

### Protocolos Suportados

#### HPGL/HPGL2 (HP Graphics Language)
- **Fabricante**: HP
- **Modelos**: DesignJet T-Series (T1200, T2300, T3500)
- **Melhor para**: Desenhos CAD, diagramas técnicos

#### PostScript Level 3
- **Fabricantes**: HP, Canon, Epson
- **Modelos**: Todos os modelos principais
- **Melhor para**: Documentos, gráficos, fotos

### Modelos Suportados

**HP DesignJet:**
- DesignJet T1200
- DesignJet T2300
- DesignJet T3500

**Canon imagePROGRAF:**
- imagePROGRAF TX-3000
- imagePROGRAF TX-4000
- imagePROGRAF PRO-6000

**Epson SureColor:**
- SureColor T5200
- SureColor T7200
- SureColor T7700

### Uso via API

```bash
# Descobrir plotters
curl http://localhost:8000/api/plotters/discover

# Obter capacidades
curl http://localhost:8000/api/plotters/{id}/capabilities

# Validar documento
curl -X POST http://localhost:8000/api/plotters/{id}/validate \
  -H "Content-Type: application/json" \
  -d '{"fileType": "PDF", "size": "A0"}'
```

---

## 🔧 Configuração

Edite `config/all_press.conf`:

```ini
[server]
port=8000
ws_port=8001

[queue]
max_workers=4

[printer]
auto_discover=true
monitor_interval=5

[logging]
level=INFO
file=all_press.log
```

---

## 🛠️ Desenvolvimento

### Executar Testes

```bash
cd build
ctest --output-on-failure
```

### Compilar com Debug

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
ninja
```

### Gerar Documentação

```bash
doxygen Doxyfile
```

### Frontend

O frontend está em `frontend/` e usa React/TypeScript com Vite.

```bash
cd frontend
npm install
npm run dev
```

A interface web estará disponível em http://localhost:3000

---

## 👥 Contribuindo

### Como Contribuir

1. **Fork o repositório**
2. **Crie uma branch** para sua feature:
   ```bash
   git checkout -b feature/minha-feature
   ```
3. **Faça suas alterações** seguindo o estilo de código
4. **Escreva testes** para novas funcionalidades
5. **Teste suas alterações**:
   ```bash
   ./scripts/build.sh
   cd build && ctest
   ```
6. **Commit suas alterações**:
   ```bash
   git commit -m "feat: adiciona nova funcionalidade X"
   ```
7. **Push para sua branch** e abra um Pull Request

### Estilo de Código

- Use C++17
- Siga as convenções de nomenclatura existentes
- Adicione comentários quando necessário
- Mantenha funções pequenas e focadas

### Conventional Commits

- `feat:` nova funcionalidade
- `fix:` correção de bug
- `docs:` documentação
- `refactor:` refatoração
- `test:` testes
- `chore:` manutenção

---

## 📝 Changelog

### [1.1.0] - 2025-01-XX

#### 🆕 Suporte Completo a Plotters
- Nova camada de protocolo para plotters
- Suporte a HPGL/HPGL2, PostScript
- Compatibilidade com HP, Canon, Epson
- Validação pré-impressão
- Seleção automática de protocolo

#### Melhorias
- Interface web React/TypeScript completa
- Endpoints de configuração do sistema
- Melhor tratamento de erros
- Logs detalhados

### [1.0.0] - 2025-11-15

#### Adicionado
- Sistema completo de gerenciamento de impressão
- Descoberta automática de impressoras
- Fila de jobs com processamento assíncrono
- Conversão automática de formatos
- API REST completa
- WebSocket para atualizações em tempo real
- Banco de dados SQLite
- Sistema de logging avançado

---

## 🐛 Troubleshooting

### CUPS não encontrado
```bash
# macOS
brew install cups

# Linux
sudo apt-get install libcups2-dev
```

### Erro de permissões
```bash
# Adicionar usuário ao grupo lpadmin
sudo usermod -a -G lpadmin $USER
```

### Porta em uso
```bash
# Modificar portas em config/all_press.conf
port=9080
ws_port=9081
```

### Verificar Logs
```bash
# Logs em tempo real
tail -f all_press.log

# Estatísticas do sistema
curl http://localhost:8000/api/system/stats
```

---

## 📞 Suporte

- **Repositório**: https://github.com/elissoncardoso1/All-Press
- **Issues**: GitHub Issues
- **Documentação**: Veja arquivos `.md` no repositório

---

## 📄 Licença

MIT License - veja LICENSE para detalhes

---

**All Press C++** - Sistema de Impressão de Alto Desempenho

*Desenvolvido com ❤️ em C++17*


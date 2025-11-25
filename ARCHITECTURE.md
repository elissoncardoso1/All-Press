# All Press C++ - Arquitetura do Sistema

## Visão Geral

All Press C++ é um sistema de gerenciamento de impressão de alto desempenho construído em C++17, projetado para ser escalável, eficiente e fácil de integrar.

## Arquitetura em Camadas

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

## Componentes Principais

### 1. API Layer

#### HTTP Server
- **Framework**: Crow (header-only C++ micro web framework)
- **Responsabilidades**:
  - Expor endpoints REST
  - Validação de requisições
  - Serialização JSON
  - Upload de arquivos
  - Tratamento de erros
- **Endpoints Principais**:
  - `/api/printers` - Gerenciamento de impressoras
  - `/api/jobs` - Gerenciamento de jobs
  - `/api/stats` - Estatísticas do sistema

#### WebSocket Server
- **Responsabilidades**:
  - Comunicação bidirecional em tempo real
  - Broadcast de eventos
  - Notificações de progresso
  - Status updates
- **Eventos**:
  - `job_created`, `job_progress`, `job_completed`
  - `printer_discovered`, `printer_status_changed`

### 2. Business Layer

#### PrinterManager
```cpp
class PrinterManager {
    // Descoberta assíncrona
    std::future<std::vector<PrinterInfo>> discover_printers_async();
    
    // CUPS integration
    std::vector<PrinterInfo> discover_cups_printers();
    
    // IPP discovery
    std::vector<PrinterInfo> discover_ipp_printers(const std::string& subnet);
    
    // Status em tempo real
    std::future<void> monitor_printer_status(const std::string& name);
};
```

**Responsabilidades**:
- Descoberta de impressoras (CUPS, IPP, mDNS)
- Monitoramento de status
- Gerenciamento de configurações
- Cache de informações

**Thread-Safety**: Mutex-protected, thread-safe operations

#### JobQueue
```cpp
class JobQueue {
    // Gerenciamento multi-threaded
    int add_job(const PrintJob& job);
    bool cancel_job(int job_id);
    bool pause_job(int job_id);
    
    // Worker pool
    void process_jobs();
};
```

**Responsabilidades**:
- Fila FIFO com priorização
- Pool de workers configurable
- Processamento assíncrono
- Controle de concorrência
- Persistência de estado

**Design Pattern**: Producer-Consumer com condition variables

#### ColorManager
```cpp
class ColorManager {
    // Conversão de espaços de cor
    std::vector<uint8_t> convert_color_space(
        const std::vector<uint8_t>& data,
        ColorSpace from, ColorSpace to);
    
    // ICC Profiles
    bool apply_icc_profile(const std::string& profile_path);
};
```

**Responsabilidades**:
- Conversão RGB ↔ CMYK
- Aplicação de ICC profiles
- Calibração de cores
- Gamma correction

### 3. Conversion Layer

#### FileProcessor
```cpp
class FileProcessor {
    // Detecção automática
    FileType detect_file_type(const std::string& filepath);
    
    // Conversão genérica
    std::string convert_to_pdf(const std::string& input_path);
};
```

**Formatos Suportados**:
- **Input**: PDF, JPG, PNG, TIFF, BMP, GIF
- **Output**: PDF (otimizado para impressão)

**Otimizações**:
- Compression
- DPI adjustment
- Color space conversion
- Page orientation

### 4. Network Layer

#### IPPClient
```cpp
class IPPClient {
    // IPP operations
    bool print_job(const std::string& printer_uri, 
                   const std::string& file_path,
                   const PrintOptions& options);
    
    // Atributos
    std::map<std::string, std::string> get_printer_attributes(
        const std::string& printer_uri);
};
```

**Protocolo**: IPP 2.0
**Operações**:
- Print-Job
- Get-Printer-Attributes
- Get-Jobs
- Cancel-Job

#### NetworkScanner
```cpp
class NetworkScanner {
    // Async scanning
    std::future<std::vector<NetworkDevice>> 
        scan_subnet_async(const std::string& subnet);
    
    // mDNS discovery
    std::future<std::vector<NetworkDevice>> 
        discover_mdns_devices_async();
};
```

**Técnicas de Descoberta**:
- ICMP ping sweep
- Port scanning (631 IPP, 9100 JetDirect)
- mDNS/Bonjour service discovery
- DNS-SD

**Performance**: Multi-threaded, até 1000 IPs/segundo

### 5. Persistence Layer

#### SQLiteManager
```cpp
class SQLiteManager {
    // Transações ACID
    bool begin_transaction();
    bool commit_transaction();
    bool rollback_transaction();
    
    // ORM-like interface
    int insert_printer(const Printer& printer);
    std::optional<Printer> get_printer(int id);
};
```

**Schema**:
```sql
CREATE TABLE printers (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE,
    uri TEXT,
    make_model TEXT,
    location TEXT,
    status INTEGER,
    created_at DATETIME
);

CREATE TABLE jobs (
    id INTEGER PRIMARY KEY,
    printer_id INTEGER,
    file_path TEXT,
    status INTEGER,
    progress REAL,
    created_at DATETIME,
    completed_at DATETIME,
    FOREIGN KEY(printer_id) REFERENCES printers(id)
);
```

## Fluxo de Dados

### 1. Descoberta de Impressoras

```
User Request
    ↓
HTTP Server → PrinterManager
    ↓                ↓
    ↓          NetworkScanner → mDNS/IPP
    ↓                ↓
    ↓          IPPClient → Query Devices
    ↓                ↓
    ↓          CUPS API → System Printers
    ↓                ↓
    ↓         ┌──────┴──────┐
    ↓         ↓             ↓
SQLiteManager     WebSocket Broadcast
    ↓                       ↓
Database             Connected Clients
```

### 2. Processamento de Job de Impressão

```
Upload File
    ↓
HTTP Server → Validate & Save
    ↓
FileProcessor → Detect Type
    ↓
    ├→ [PDF] → Direct
    ├→ [Image] → ImageProcessor → PDF
    └→ [Other] → Convert → PDF
    ↓
ColorManager → Apply ICC Profile
    ↓
JobQueue → Add to Queue
    ↓
Worker Thread → Process Job
    ↓
    ├→ Update Progress (WebSocket)
    ├→ Save to Database
    └→ Send to Printer (IPPClient/CUPS)
    ↓
Complete → Notify Client
```

## Concorrência e Threading

### Thread Pool Architecture

```
Main Thread
    ↓
    ├─→ HTTP Server Thread (Crow)
    ├─→ WebSocket Server Thread
    ├─→ Job Queue Workers (configurable N threads)
    ├─→ Network Scanner Workers (configurable M threads)
    └─→ Printer Monitor Thread
```

### Sincronização

- **Mutexes**: Proteção de estruturas compartilhadas
- **Condition Variables**: Producer-consumer patterns
- **Atomic Operations**: Contadores e flags
- **Lock Guards**: RAII para exception safety

### Thread Safety Garantias

- `PrinterManager`: Thread-safe com mutex interno
- `JobQueue`: Lock-free queue com atomic operations
- `SQLiteManager`: Serialized access com mutex
- `WebSocketServer`: Thread-safe broadcast

## Performance Considerations

### Otimizações Implementadas

1. **Memory Management**
   - Smart pointers (unique_ptr, shared_ptr)
   - RAII patterns
   - Move semantics para objetos grandes

2. **I/O Operations**
   - Async file operations
   - Memory-mapped files para PDFs grandes
   - Buffered I/O

3. **Network**
   - Connection pooling
   - Keep-alive connections
   - Parallel requests

4. **Database**
   - Prepared statements
   - Batch inserts
   - Indexing estratégico

### Benchmarks Esperados

- **Throughput**: 100+ jobs/minuto
- **Latency**: <100ms para operações simples
- **Memory**: ~50MB base + ~10MB por job ativo
- **CPU**: 1-2 cores em uso normal

## Escalabilidade

### Horizontal Scaling

```
Load Balancer
    ↓
┌───┴───┬───────┬───────┐
│ App 1 │ App 2 │ App 3 │
└───┬───┴───┬───┴───┬───┘
    └───────┼───────┘
            ↓
    Shared Database
```

### Vertical Scaling

- Aumentar `max_concurrent_jobs`
- Aumentar workers do NetworkScanner
- Aumentar pool de conexões

## Segurança

### Medidas Implementadas

1. **Input Validation**
   - File type verification
   - Size limits
   - Path sanitization

2. **Error Handling**
   - Exception safety
   - Graceful degradation
   - Detailed logging

3. **Resource Limits**
   - Max file size
   - Max concurrent jobs
   - Timeout configurations

### Futuras Melhorias

- Authentication/Authorization
- TLS/SSL para WebSocket
- Rate limiting
- Audit logging

## Deployment

### Standalone
```bash
./all_press_c
```

### Docker
```bash
docker-compose up -d
```

### Systemd Service
```ini
[Unit]
Description=All Press C++ Print Server
After=cups.service

[Service]
Type=simple
ExecStart=/usr/local/bin/all_press_c
Restart=always

[Install]
WantedBy=multi-user.target
```

## Monitoring

### Métricas Disponíveis

- Jobs processados
- Taxa de sucesso/falha
- Tempo médio de processamento
- Impressoras ativas
- Uso de recursos

### Logging

```
[2025-11-15 10:30:45] [INFO] Server started on port 8080
[2025-11-15 10:30:46] [INFO] Discovered printer: HP_LaserJet
[2025-11-15 10:31:00] [INFO] Job 123 created for printer HP_LaserJet
[2025-11-15 10:31:05] [INFO] Job 123 progress: 50%
[2025-11-15 10:31:10] [INFO] Job 123 completed successfully
```

## Tecnologias e Dependências

| Componente | Tecnologia | Versão |
|------------|-----------|--------|
| Language | C++ | 17 |
| Build System | CMake | 3.20+ |
| Package Manager | Conan | 2.0+ |
| HTTP Framework | Crow | 1.0+ |
| Database | SQLite | 3.x |
| Image Processing | ImageMagick | 7.x |
| PDF Processing | Poppler | 22.x |
| JSON | nlohmann/json | 3.x |
| Testing | Google Test | 1.12+ |
| Logging | spdlog | 1.x |

## Roadmap

### v1.1.0 (Q1 2026)
- Interface web de administração
- Suporte para Windows
- Métricas com Prometheus

### v2.0.0 (Q2 2026)
- Clustering
- Machine learning para otimização
- GraphQL API

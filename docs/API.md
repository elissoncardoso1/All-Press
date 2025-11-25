# All Press C++ - API Documentation

## Visão Geral

A API REST do All Press C++ fornece endpoints para gerenciar impressoras, jobs de impressão, configurações do sistema e muito mais.

**Base URL**: `http://localhost:8000/api`

**Versão**: 1.1.0

## Autenticação

Atualmente, a API não requer autenticação. Em versões futuras, será implementado JWT.

## Endpoints

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

**Resposta**: Array de impressoras (mesmo formato do GET)

#### GET /api/printers/{id}
Obtém detalhes de uma impressora específica.

**Parâmetros**:
- `id` (path): ID da impressora

**Resposta**: Objeto Printer (mesmo formato do array acima)

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
    "startedAt": "2025-01-15T10:30:05Z",
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

**Exemplo de opções**:
```json
{
  "copies": 2,
  "colorMode": "color",
  "duplex": "long-edge",
  "paperSize": "A4",
  "quality": "high"
}
```

**Resposta**: Objeto PrintJob

#### GET /api/jobs/{id}
Obtém detalhes de um job específico.

**Parâmetros**:
- `id` (path): ID do job

**Resposta**: Objeto PrintJob

#### POST /api/jobs/{id}/cancel
Cancela um job de impressão.

**Parâmetros**:
- `id` (path): ID do job

**Resposta**:
```json
{
  "success": true,
  "message": "Job cancelled successfully"
}
```

#### POST /api/jobs/{id}/retry
Tenta novamente um job que falhou.

**Parâmetros**:
- `id` (path): ID do job

**Resposta**:
```json
{
  "success": true,
  "message": "Job queued for retry"
}
```

### System

#### GET /api/system/status
Obtém o status do sistema.

**Resposta**:
```json
{
  "status": "operational",
  "uptime": 12345,
  "version": "1.1.0"
}
```

#### GET /api/system/stats
Obtém estatísticas do sistema.

**Resposta**:
```json
{
  "printersOnline": 3,
  "printersTotal": 5,
  "jobsPending": 2,
  "jobsProcessing": 1,
  "jobsCompleted": 150,
  "jobsFailed": 3,
  "pagesTotal": 5000,
  "pagesToday": 150,
  "estimatedCost": 25.50
}
```

#### GET /api/system/metrics
Obtém métricas de recursos do sistema.

**Resposta**:
```json
{
  "cpuUsage": 15.5,
  "memoryUsage": 45.2,
  "diskUsage": 60.0,
  "temperature": 42.0
}
```

#### GET /api/system/settings
Obtém todas as configurações do sistema.

**Resposta**:
```json
{
  "server": {
    "port": 8000,
    "ws_port": 8001
  },
  "queue": {
    "max_workers": 4
  },
  "printer": {
    "auto_discover": true,
    "discover_interval": 30
  },
  "color": {
    "default_input_profile": "sRGB",
    "enable_calibration": true
  },
  "database": {
    "path": "all_press.db"
  },
  "logging": {
    "level": "INFO",
    "file": "all_press.log"
  }
}
```

#### POST /api/system/settings
Atualiza as configurações do sistema.

**Content-Type**: `application/json`

**Body**: Objeto Settings (mesmo formato do GET)

**Resposta**:
```json
{
  "success": true,
  "message": "Settings saved successfully"
}
```

#### GET /api/system/logs
Obtém logs do sistema.

**Parâmetros de Query**:
- `limit` (opcional): Número máximo de logs a retornar

**Resposta**:
```json
[
  {
    "timestamp": "2025-01-15T10:30:00Z",
    "level": "INFO",
    "message": "System started"
  }
]
```

## Códigos de Status HTTP

- `200 OK`: Requisição bem-sucedida
- `400 Bad Request`: Requisição inválida
- `404 Not Found`: Recurso não encontrado
- `500 Internal Server Error`: Erro interno do servidor

## Tratamento de Erros

Todos os erros retornam um objeto JSON no seguinte formato:

```json
{
  "error": "Mensagem de erro descritiva",
  "success": false
}
```

## Exemplos de Uso

### cURL

#### Listar impressoras
```bash
curl http://localhost:8000/api/printers
```

#### Criar job de impressão
```bash
curl -X POST http://localhost:8000/api/jobs \
  -F "file=@document.pdf" \
  -F "printer_id=HP_LaserJet" \
  -F 'options={"copies":2,"colorMode":"color"}'
```

#### Obter configurações
```bash
curl http://localhost:8000/api/system/settings
```

#### Atualizar configurações
```bash
curl -X POST http://localhost:8000/api/system/settings \
  -H "Content-Type: application/json" \
  -d '{
    "server": {
      "port": 9000,
      "ws_port": 9001
    }
  }'
```

### JavaScript/TypeScript

```typescript
import axios from 'axios';

const api = axios.create({
  baseURL: 'http://localhost:8000/api'
});

// Listar impressoras
const printers = await api.get('/printers');

// Criar job
const formData = new FormData();
formData.append('file', file);
formData.append('printer_id', 'HP_LaserJet');
formData.append('options', JSON.stringify({ copies: 2 }));

const job = await api.post('/jobs', formData);

// Obter configurações
const settings = await api.get('/system/settings');

// Salvar configurações
await api.post('/system/settings', {
  server: { port: 9000 }
});
```

### Python

```python
import requests

base_url = "http://localhost:8000/api"

# Listar impressoras
printers = requests.get(f"{base_url}/printers").json()

# Criar job
with open("document.pdf", "rb") as f:
    files = {"file": f}
    data = {
        "printer_id": "HP_LaserJet",
        "options": '{"copies": 2}'
    }
    job = requests.post(f"{base_url}/jobs", files=files, data=data).json()

# Obter configurações
settings = requests.get(f"{base_url}/system/settings").json()

# Salvar configurações
requests.post(f"{base_url}/system/settings", json={
    "server": {"port": 9000}
})
```

## WebSocket

O servidor também fornece um WebSocket na porta 8001 (configurável) para atualizações em tempo real.

**URL**: `ws://localhost:8001`

### Eventos

- `job_created`: Novo job criado
- `job_progress`: Progresso de um job atualizado
- `job_completed`: Job concluído
- `job_failed`: Job falhou
- `printer_discovered`: Nova impressora descoberta
- `printer_status_changed`: Status de impressora mudou

### Exemplo de Mensagem

```json
{
  "type": "job_progress",
  "data": {
    "job_id": "job_123",
    "progress": 50.0
  }
}
```

## Especificação OpenAPI

Uma especificação OpenAPI completa está disponível em `docs/openapi.yaml`. Você pode usar ferramentas como Swagger UI ou Postman para importar e testar a API.

## Limitações e Notas

- O tamanho máximo de arquivo é determinado pela configuração do servidor
- Alguns endpoints podem retornar dados mockados em desenvolvimento
- A API está sujeita a mudanças em versões futuras

## Suporte

Para mais informações, consulte:
- README.md
- ARCHITECTURE.md
- PROXIMOS_PASSOS.md


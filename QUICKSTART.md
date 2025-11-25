# All Press C++ - Guia de Início Rápido

## Instalação Rápida

### macOS

```bash
# Instalar dependências e build em um comando
./scripts/install.sh
```

### Linux (Ubuntu/Debian)

```bash
# Instalar dependências e build em um comando
./scripts/install.sh
```

### Build Manual

```bash
# 1. Instalar dependências do Conan
conan install . --output-folder=build --build=missing -s build_type=Release

# 2. Configurar com CMake
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

# 3. Compilar
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

# 4. Executar
./all_press_c
```

## Uso Rápido

### Modo Desenvolvimento

```bash
# Inicia o servidor com hot-reload
./scripts/dev.sh
```

### Iniciar o Servidor

```bash
cd build
./all_press_c
```

O servidor iniciará nas seguintes portas:
- **API REST**: http://localhost:8080
- **WebSocket**: ws://localhost:8081

### API Endpoints Principais

#### Listar Impressoras
```bash
curl http://localhost:8080/api/printers
```

#### Descobrir Impressoras na Rede
```bash
curl -X POST http://localhost:8080/api/printers/discover
```

#### Criar Job de Impressão
```bash
curl -X POST http://localhost:8080/api/jobs \
  -F "file=@document.pdf" \
  -F "printer=HP_LaserJet" \
  -F "copies=2" \
  -F "color_mode=color"
```

#### Status de um Job
```bash
curl http://localhost:8080/api/jobs/123
```

#### Cancelar Job
```bash
curl -X DELETE http://localhost:8080/api/jobs/123
```

### WebSocket - Atualizações em Tempo Real

```javascript
const ws = new WebSocket('ws://localhost:8081');

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

## Configuração

Edite `config/all_press.conf`:

```ini
[server]
http_port = 8080
websocket_port = 8081
max_workers = 4

[printing]
max_concurrent_jobs = 10
auto_discovery = true
discovery_interval = 300

[database]
path = ./all_press.db

[logging]
level = info
file = ./logs/all_press.log
```

## Estrutura de Dados

### PrintJob
```json
{
  "job_id": 123,
  "printer_name": "HP_LaserJet",
  "file_path": "/tmp/document.pdf",
  "status": "Printing",
  "progress": 0.75,
  "options": {
    "copies": 2,
    "color_mode": "color",
    "duplex": "long-edge",
    "media_size": "A4"
  }
}
```

### PrinterInfo
```json
{
  "name": "HP_LaserJet",
  "uri": "ipp://192.168.1.100:631/ipp/print",
  "make_model": "HP LaserJet Pro M404",
  "status": "idle",
  "is_online": true,
  "jobs_count": 3
}
```

## Testes

```bash
cd build
ctest --output-on-failure
```

## Troubleshooting

### Erro: "CUPS not found"
```bash
# macOS
brew install cups

# Linux
sudo apt-get install libcups2-dev
```

### Erro: "Permission denied" ao acessar impressora
```bash
# Adicionar usuário ao grupo lpadmin
sudo usermod -a -G lpadmin $USER
```

### Porta em uso
```bash
# Modificar portas em config/all_press.conf
http_port = 9080
websocket_port = 9081
```

## Performance

### Otimizações Recomendadas

1. **Concorrência**: Ajuste `max_concurrent_jobs` baseado em CPU cores
2. **Cache**: Habilite cache de conversão para arquivos grandes
3. **Rede**: Use descoberta manual para redes grandes

### Monitoramento

```bash
# Logs em tempo real
tail -f logs/all_press.log

# Estatísticas do sistema
curl http://localhost:8080/api/stats
```

## Próximos Passos

1. Configure suas impressoras em `config/all_press.conf`
2. Execute descoberta automática de rede
3. Teste com um documento de amostra
4. Integre com sua aplicação via API REST ou WebSocket

## Suporte

- **Documentação**: [README.md](README.md)
- **Issues**: GitHub Issues
- **Logs**: `logs/all_press.log`


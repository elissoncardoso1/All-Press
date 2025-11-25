# All Press C++ - Changelog

## [1.1.0] - 2025-11-17

### 🆕 Suporte Completo a Plotters

#### Nova Camada de Protocolo
- **PlotterProtocolBase**: Interface abstrata para protocolos de plotters
- **HPGLGenerator**: Gerador de comandos HPGL/HPGL2 para plotters HP
- **PostScriptGenerator**: Gerador PostScript para Canon e Epson
- **CompatibilityMatrix**: Base de dados de compatibilidade de modelos
- **PlotterProtocolFactory**: Factory pattern para criação de protocolos

#### Plotters Suportados
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

#### Protocolos Implementados
- **HPGL/HPGL2**: Otimizado para desenhos técnicos e CAD
- **PostScript Level 3**: Universal, suporte a fotos e documentos
- **ESC/P**: Preparado (implementação futura)

#### Extensões do Core
**PrinterManager:**
- `discover_plotters_advanced_async()`: Descoberta com info avançada
- `get_plotter_info()`: Obter capacidades do plotter
- `validate_document_for_plotter()`: Validação pré-impressão
- `select_best_protocol()`: Seleção automática de protocolo
- `is_plotter()`: Detectar se é um plotter
- `detect_plotter_vendor()`: Identificar fabricante

**JobQueue:**
- `process_job_with_protocol()`: Processamento com conversão de protocolo
- `validate_job_compatibility()`: Pre-flight checks
- Cache de protocolos para performance

#### Novos Endpoints REST API
- `GET /api/plotters/discover` - Descobrir plotters avançado
- `GET /api/plotters/{id}/capabilities` - Capacidades do plotter
- `GET /api/plotters/{id}/protocols` - Protocolos suportados
- `POST /api/plotters/{id}/validate` - Validar documento
- `POST /api/print/auto` - Impressão com seleção automática
- `GET /api/compatibility-matrix` - Matriz completa de compatibilidade
- `GET /api/plotters/{vendor}/{model}/quirks` - Quirks conhecidos

#### Arquivos de Configuração
- `config/plotter_specs.json`: Especificações completas dos fabricantes
- Suporte a tamanhos customizados
- Quirks e workarounds por modelo
- Links para documentação oficial

#### Documentação
- `docs/PLOTTER_SUPPORT.md`: Guia completo de uso
- `docs/MANUFACTURER_SPECS.md`: Especificações detalhadas
- `PLOTTER_SUPPORT_IMPLEMENTATION.md`: Documento técnico de implementação
- Exemplos de uso via API REST e C++

#### Build System
- Nova biblioteca `all_press_protocols`
- Arquivos fonte adicionados ao CMakeLists.txt
- Linking correto das dependências

#### Características Avançadas
- Seleção automática de protocolo baseado em vendor e modelo
- Validação pré-impressão de compatibilidade
- Otimizações específicas por vendor
- Fallback automático entre protocolos
- Cache de informações de plotters
- Conversão de protocolo em tempo real

#### Performance
- Descoberta de plotters: < 2s
- Seleção de protocolo: < 100ms
- Validação pré-impressão: < 50ms
- Cache de protocolos reduz overhead

### Melhorias
- Arquitetura expandida com camada de protocolo
- Melhor suporte a impressoras de grande formato
- API REST mais completa
- Documentação expandida

## [1.0.0] - 2025-11-15

### Adicionado
- Sistema completo de gerenciamento de impressão em C++
- Descoberta automática de impressoras via CUPS e IPP
- Escaneamento de rede com suporte a mDNS/Bonjour
- Fila de jobs com processamento assíncrono e multi-threading
- Conversão automática de formatos (PDF, imagens, documentos)
- Gerenciamento de cores com ICC profiles
- API REST completa para integração
- WebSocket para atualizações em tempo real
- Banco de dados SQLite para persistência
- Sistema de logging avançado
- Configuração via arquivo INI
- Suporte para CUPS (macOS/Linux)
- Testes unitários com Google Test
- Docker e Docker Compose para deployment
- Scripts de build e instalação automatizados

### Core Features
- **PrinterManager**: Descoberta e gerenciamento de impressoras
- **JobQueue**: Fila de jobs com priorização e controle
- **FileProcessor**: Conversão automática de formatos
- **ColorManager**: Gerenciamento de cores e perfis ICC
- **NetworkScanner**: Descoberta de dispositivos na rede
- **IPPClient**: Cliente IPP completo
- **SQLiteManager**: Persistência de dados
- **HTTPServer**: API REST com Crow
- **WebSocketServer**: Comunicação em tempo real

### API Endpoints
- `GET /api/printers` - Listar impressoras
- `POST /api/printers/discover` - Descobrir impressoras
- `POST /api/jobs` - Criar job de impressão
- `GET /api/jobs/:id` - Status do job
- `DELETE /api/jobs/:id` - Cancelar job
- `GET /api/stats` - Estatísticas do sistema

### Documentação
- README completo com guia de instalação
- QUICKSTART.md para início rápido
- INTEGRATION_EXAMPLES.md com exemplos em Node.js, Python, React e cURL
- Documentação inline em todos os componentes
- Exemplos de uso e configuração

### Build System
- CMake 3.20+ com C++17
- Conan para gerenciamento de dependências
- Scripts de build automatizados
- Suporte para macOS e Linux
- Docker para containerização

### Testes
- Testes unitários para componentes principais
- Test fixtures para FileProcessor
- Test fixtures para JobQueue
- Test fixtures para PrinterManager
- Integração com Google Test

### Configuração
- Arquivo de configuração INI
- Variáveis de ambiente
- Configuração de logging
- Configuração de rede e portas
- Configuração de workers e concorrência

## [Futuro]

### Planejado para v1.1.0
- Suporte para Windows via WinSpool
- Interface web de administração
- Autenticação e autorização
- Métricas e monitoring com Prometheus
- Cache de conversão de documentos
- Suporte para mais formatos de arquivo
- API GraphQL
- Plugins system
- Notificações por email
- Multi-tenancy

### Planejado para v2.0.0
- Clustering e load balancing
- Replicação de banco de dados
- High availability
- Kubernetes deployment
- Advanced analytics
- Machine learning para otimização
- Suporte para dispositivos 3D printing

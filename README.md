# All Press C++ - High-Performance Print Management System

![Version](https://img.shields.io/badge/version-1.1.0-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17-00599C)
![License](https://img.shields.io/badge/license-MIT-green)

Sistema completo de gerenciamento de impressão de alta performance desenvolvido em C/C++ com APIs nativas. Projetado para gráficas rápidas que necessitam de máximo desempenho, robustez e controle granular sobre operações de impressão.

## 🚀 Início Rápido

### Instalação

```bash
# Clone o repositório
git clone https://github.com/elissoncardoso1/All-Press.git
cd All-Press

# Instalar e compilar
./scripts/install.sh

# Iniciar servidor
cd build && ./all_press_server
```

### Uso Básico

```bash
# Listar impressoras
curl http://localhost:8000/api/printers

# Imprimir documento
curl -X POST http://localhost:8000/api/jobs \
  -F "file=@document.pdf" \
  -F "printer_id=HP_LaserJet" \
  -F 'options={"copies":2}'
```

## ✨ Características Principais

- **Performance**: 10-50x mais rápido que soluções Python
- **Memória**: 80-90% menos uso de memória
- **Descoberta Automática**: CUPS, IPP, mDNS/Bonjour
- **Multi-threading**: Processamento assíncrono e paralelo
- **API REST**: Endpoints completos para integração
- **WebSocket**: Atualizações em tempo real
- **Suporte a Plotters**: HP, Canon, Epson (HPGL, PostScript)
- **Interface Web**: React/TypeScript moderna e responsiva

## 📚 Documentação

📖 **[Documentação Completa](DOCUMENTACAO.md)** - Guia completo com todos os detalhes

- [Arquitetura](ARCHITECTURE.md) - Estrutura e componentes do sistema
- [API REST](docs/API.md) - Documentação da API
- [Exemplos de Integração](INTEGRATION_EXAMPLES.md) - Node.js, Python, cURL
- [Suporte a Plotters](docs/PLOTTER_SUPPORT.md) - Guia de uso de plotters
- [Contribuindo](CONTRIBUTING.md) - Como contribuir para o projeto

## 🏗️ Estrutura do Projeto

```
All-Press/
├── src/              # Código-fonte C++
├── include/          # Headers públicos
├── frontend/         # Interface web React/TypeScript
├── tests/            # Testes unitários
├── config/           # Arquivos de configuração
├── docs/             # Documentação técnica
└── scripts/          # Scripts de build e instalação
```

## 🔧 Requisitos

### macOS
```bash
brew install cmake ninja boost openssl sqlite3 cups
```

### Ubuntu/Debian
```bash
sudo apt-get install -y \
    build-essential cmake ninja-build \
    libcups2-dev libboost-all-dev libssl-dev \
    libsqlite3-dev libcurl4-openssl-dev
```

## 🚀 Performance

| Operação | Python/FastAPI | C++ Nativo | Melhoria |
|----------|---------------|------------|----------|
| Descoberta de rede (254 IPs) | 2-5 seg | 0.3-0.8 seg | **10-15x** |
| Processamento PDF 100MB | 45-60 seg | 8-15 seg | **4-6x** |
| Uso de memória base | 150-300 MB | 15-30 MB | **80-90%** |
| Jobs simultâneos | 10-20/min | 50-100/min | **5x** |

## 🖨️ Suporte a Plotters

- **HP DesignJet**: T1200, T2300, T3500 (HPGL/HPGL2)
- **Canon imagePROGRAF**: TX-3000, TX-4000, PRO-6000 (PostScript)
- **Epson SureColor**: T5200, T7200, T7700 (PostScript)

## 🌐 API REST

**Base URL**: `http://localhost:8000/api`

Principais endpoints:
- `GET /api/printers` - Listar impressoras
- `POST /api/jobs` - Criar job de impressão
- `GET /api/jobs/{id}` - Status do job
- `GET /api/system/status` - Status do sistema
- `GET /api/plotters/discover` - Descobrir plotters

Veja a [documentação completa da API](docs/API.md) para mais detalhes.

## 🧪 Testes

```bash
cd build
ctest --output-on-failure
```

## 🐳 Docker

```bash
docker-compose up -d
```

## 📝 Licença

MIT License - veja [LICENSE](LICENSE) para detalhes

## 👥 Contribuindo

Contribuições são bem-vindas! Veja [CONTRIBUTING.md](CONTRIBUTING.md) para diretrizes.

## 📞 Suporte

- **Repositório**: https://github.com/elissoncardoso1/All-Press
- **Issues**: GitHub Issues
- **Documentação**: [DOCUMENTACAO.md](DOCUMENTACAO.md)

---

**All Press C++** - Sistema de Impressão de Alto Desempenho

*Desenvolvido com ❤️ em C++17*

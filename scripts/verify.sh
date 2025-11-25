#!/bin/bash

# All Press C++ - Sistema de Verificação
# Verifica se todos os componentes estão presentes e corretos

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=================================="
echo "All Press C++ - Verificação"
echo "=================================="
echo ""

# Função para verificar arquivo
check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} $1"
        return 0
    else
        echo -e "${RED}✗${NC} $1 (faltando)"
        return 1
    fi
}

# Função para verificar diretório
check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} $1/"
        return 0
    else
        echo -e "${RED}✗${NC} $1/ (faltando)"
        return 1
    fi
}

ERRORS=0

# Verificar arquivos principais
echo "📁 Arquivos de Configuração:"
check_file "CMakeLists.txt" || ((ERRORS++))
check_file "conanfile.txt" || ((ERRORS++))
check_file "config/all_press.conf" || ((ERRORS++))
echo ""

# Verificar headers
echo "📄 Headers (include/):"
check_file "include/core/printer_manager.h" || ((ERRORS++))
check_file "include/core/job_queue.h" || ((ERRORS++))
check_file "include/core/color_manager.h" || ((ERRORS++))
check_file "include/network/ipp_client.h" || ((ERRORS++))
check_file "include/network/network_scanner.h" || ((ERRORS++))
check_file "include/conversion/file_processor.h" || ((ERRORS++))
check_file "include/database/sqlite_manager.h" || ((ERRORS++))
check_file "include/utils/config.h" || ((ERRORS++))
check_file "include/utils/logger.h" || ((ERRORS++))
check_file "include/utils/file_utils.h" || ((ERRORS++))
echo ""

# Verificar implementações
echo "⚙️  Implementações (src/):"
check_file "src/main.cpp" || ((ERRORS++))
check_file "src/core/printer_manager.cpp" || ((ERRORS++))
check_file "src/core/job_queue.cpp" || ((ERRORS++))
check_file "src/core/color_manager.cpp" || ((ERRORS++))
check_file "src/network/ipp_client.cpp" || ((ERRORS++))
check_file "src/network/network_scanner.cpp" || ((ERRORS++))
check_file "src/conversion/file_processor.cpp" || ((ERRORS++))
check_file "src/database/sqlite_manager.cpp" || ((ERRORS++))
check_file "src/api/http_server.cpp" || ((ERRORS++))
check_file "src/api/websocket_server.cpp" || ((ERRORS++))
echo ""

# Verificar testes
echo "🧪 Testes:"
check_file "tests/CMakeLists.txt" || ((ERRORS++))
check_file "tests/test_file_processor.cpp" || ((ERRORS++))
check_file "tests/test_job_queue.cpp" || ((ERRORS++))
check_file "tests/test_printer_manager.cpp" || ((ERRORS++))
echo ""

# Verificar scripts
echo "📜 Scripts:"
check_file "scripts/build.sh" || ((ERRORS++))
check_file "scripts/dev.sh" || ((ERRORS++))
check_file "scripts/install.sh" || ((ERRORS++))

# Verificar permissões de execução
if [ -x "scripts/build.sh" ]; then
    echo -e "${GREEN}✓${NC} scripts/build.sh (executável)"
else
    echo -e "${YELLOW}⚠${NC} scripts/build.sh (não executável)"
fi

if [ -x "scripts/dev.sh" ]; then
    echo -e "${GREEN}✓${NC} scripts/dev.sh (executável)"
else
    echo -e "${YELLOW}⚠${NC} scripts/dev.sh (não executável)"
fi

if [ -x "scripts/install.sh" ]; then
    echo -e "${GREEN}✓${NC} scripts/install.sh (executável)"
else
    echo -e "${YELLOW}⚠${NC} scripts/install.sh (não executável)"
fi
echo ""

# Verificar documentação
echo "📚 Documentação:"
check_file "README.md" || ((ERRORS++))
check_file "QUICKSTART.md" || ((ERRORS++))
check_file "ARCHITECTURE.md" || ((ERRORS++))
check_file "INTEGRATION_EXAMPLES.md" || ((ERRORS++))
check_file "CONTRIBUTING.md" || ((ERRORS++))
check_file "CHANGELOG.md" || ((ERRORS++))
check_file "PROJECT_SUMMARY.md" || ((ERRORS++))
check_file "LICENSE" || ((ERRORS++))
echo ""

# Verificar Docker
echo "🐳 Docker:"
check_file "Dockerfile" || ((ERRORS++))
check_file "docker-compose.yml" || ((ERRORS++))
echo ""

# Contar arquivos
echo "📊 Estatísticas:"
HEADER_COUNT=$(find include -name "*.h" 2>/dev/null | wc -l | xargs)
CPP_COUNT=$(find src -name "*.cpp" 2>/dev/null | wc -l | xargs)
TEST_COUNT=$(find tests -name "*.cpp" 2>/dev/null | wc -l | xargs)
TOTAL_FILES=$((HEADER_COUNT + CPP_COUNT + TEST_COUNT))

echo "  - Headers: $HEADER_COUNT"
echo "  - Implementações: $CPP_COUNT"
echo "  - Testes: $TEST_COUNT"
echo "  - Total: $TOTAL_FILES arquivos"

# Contar linhas de código
if command -v wc &> /dev/null; then
    TOTAL_LINES=$(find . -name "*.cpp" -o -name "*.h" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}')
    echo "  - Linhas de código: ~$TOTAL_LINES"
fi
echo ""

# Resumo
echo "=================================="
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✅ Todos os componentes estão presentes!${NC}"
    echo ""
    echo "Próximos passos:"
    echo "  1. ./scripts/install.sh    # Instalar dependências e compilar"
    echo "  2. ./scripts/dev.sh        # Iniciar em modo desenvolvimento"
    echo "  3. Acessar http://localhost:8080"
else
    echo -e "${RED}❌ Encontrados $ERRORS erros${NC}"
    echo "Alguns arquivos estão faltando. Verifique a estrutura do projeto."
fi
echo "=================================="

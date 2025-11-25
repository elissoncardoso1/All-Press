#!/bin/bash

echo "=========================================="
echo "🖨️  TESTE DE STATUS DE IMPRESSORAS"
echo "=========================================="
echo ""

# Cores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 1. Verificar se o CUPS está rodando
echo "1️⃣  Verificando CUPS..."
if pgrep -x "cupsd" > /dev/null; then
    echo -e "${GREEN}✓${NC} CUPS está rodando"
else
    echo -e "${RED}✗${NC} CUPS NÃO está rodando"
    echo "   Execute: sudo systemctl start cups (Linux) ou sudo launchctl load /System/Library/LaunchDaemons/org.cups.cupsd.plist (macOS)"
    exit 1
fi
echo ""

# 2. Listar impressoras configuradas
echo "2️⃣  Impressoras Configuradas no CUPS:"
echo ""
PRINTERS=$(lpstat -p 2>/dev/null | grep -E "^printer" | awk '{print $2}')

if [ -z "$PRINTERS" ]; then
    echo -e "${YELLOW}⚠${NC}  Nenhuma impressora configurada"
    exit 0
fi

for PRINTER in $PRINTERS; do
    echo -e "${BLUE}━━━ $PRINTER ━━━${NC}"
    
    # Status do CUPS
    STATUS=$(lpstat -p "$PRINTER" 2>/dev/null | head -1)
    if echo "$STATUS" | grep -q "idle"; then
        echo -e "   CUPS Status: ${GREEN}IDLE (pronta)${NC}"
    elif echo "$STATUS" | grep -q "processing"; then
        echo -e "   CUPS Status: ${GREEN}PROCESSING (imprimindo)${NC}"
    elif echo "$STATUS" | grep -q "disabled"; then
        echo -e "   CUPS Status: ${RED}DISABLED (desabilitada)${NC}"
    else
        echo -e "   CUPS Status: ${RED}STOPPED (parada)${NC}"
    fi
    
    # URI
    URI=$(lpstat -v "$PRINTER" 2>/dev/null | grep -oE "(ipp|http|socket|lpd|usb|serial)://[^ ]+")
    echo "   URI: $URI"
    
    # Tipo e teste de conectividade
    if [[ "$URI" =~ ^(ipp|http|socket|lpd):// ]]; then
        # Impressora de REDE
        HOST=$(echo "$URI" | sed -E 's#^[a-z]+://([^/:]+).*#\1#')
        PORT=$(echo "$URI" | grep -oE ":[0-9]+" | tr -d ":" || echo "631")
        
        echo -e "   Tipo: ${YELLOW}REDE${NC}"
        echo "   Host: $HOST:$PORT"
        echo -n "   Conectividade: "
        
        # Tentar conectar (timeout de 2 segundos)
        if nc -z -w 2 "$HOST" "$PORT" 2>/dev/null; then
            echo -e "${GREEN}✓ ONLINE (conectável)${NC}"
        else
            echo -e "${RED}✗ OFFLINE (não conectável)${NC}"
        fi
    elif [[ "$URI" =~ ^usb:// ]]; then
        echo -e "   Tipo: ${GREEN}USB${NC}"
        echo "   Conectividade: Depende do CUPS"
    elif [[ "$URI" =~ ^(serial|parallel):// ]]; then
        echo -e "   Tipo: ${GREEN}LOCAL (Serial/Paralela)${NC}"
        echo "   Conectividade: Depende do CUPS"
    elif [[ "$URI" =~ ^(file|cups-pdf):// ]]; then
        echo -e "   Tipo: ${BLUE}VIRTUAL${NC}"
        echo "   Conectividade: Sempre online"
    else
        echo "   Tipo: DESCONHECIDO"
    fi
    
    echo ""
done

# 3. Testar endpoint da API (se o servidor estiver rodando)
echo "3️⃣  Testando API do Servidor..."
echo ""

if curl -s --connect-timeout 2 http://localhost:8001/api/printers > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Servidor está rodando em http://localhost:8001"
    echo ""
    echo "📡 Resposta da API:"
    echo ""
    
    # Buscar e formatar resposta
    RESPONSE=$(curl -s http://localhost:8001/api/printers)
    
    if command -v jq &> /dev/null; then
        echo "$RESPONSE" | jq -r '.[] | "   🖨️  \(.name): \(.status) (\(if .status == "online" then "🟢" else "🔴" end))"'
    else
        echo "$RESPONSE" | python3 -m json.tool 2>/dev/null || echo "$RESPONSE"
    fi
else
    echo -e "${YELLOW}⚠${NC}  Servidor NÃO está rodando"
    echo "   Execute: cd build && ./all_press_server"
fi

echo ""
echo "=========================================="
echo "✅ Teste Concluído"
echo "=========================================="
echo ""
echo "💡 Dicas:"
echo "   - Para desabilitar uma impressora: cupsdisable NOME"
echo "   - Para habilitar uma impressora: cupsenable NOME"
echo "   - Para ver logs do servidor: tail -f logs/all_press.log"
echo "   - Para simular offline: sudo route add -host IP 127.0.0.1"
echo ""


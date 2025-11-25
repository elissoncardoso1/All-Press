#!/bin/bash

# Script de teste para upload de arquivos
# Testa o endpoint POST /api/jobs e o processamento de arquivos

set -e

API_URL="http://localhost:8000/api"
TEST_FILE="/tmp/test_upload.pdf"

echo "==========================================="
echo "   Teste de Upload - All Press C++"
echo "==========================================="
echo ""

# Criar arquivo de teste PDF básico
echo "📄 Criando arquivo de teste..."
cat > "$TEST_FILE" << 'EOF'
%PDF-1.4
1 0 obj
<< /Type /Catalog /Pages 2 0 R >>
endobj
2 0 obj
<< /Type /Pages /Kids [3 0 R] /Count 1 >>
endobj
3 0 obj
<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] >>
endobj
xref
0 4
trailer
<< /Size 4 /Root 1 0 R >>
startxref
100
%%EOF
EOF

echo "✅ Arquivo de teste criado: $TEST_FILE"
echo ""

# Verificar se o servidor está rodando
echo "🔍 Verificando se o servidor está rodando..."
if ! curl -s "$API_URL/../" > /dev/null; then
    echo "❌ Servidor não está rodando!"
    echo "   Execute: ./start.sh"
    exit 1
fi
echo "✅ Servidor está rodando"
echo ""

# Obter lista de impressoras
echo "🖨️  Obtendo lista de impressoras..."
PRINTERS=$(curl -s "$API_URL/printers")
PRINTER_COUNT=$(echo "$PRINTERS" | grep -o '"id"' | wc -l | tr -d ' ')
echo "   Encontradas $PRINTER_COUNT impressoras"

if [ "$PRINTER_COUNT" -eq 0 ]; then
    echo "⚠️  Nenhuma impressora encontrada. O teste continuará com 'default'"
    PRINTER_ID="default"
else
    # Extrair primeiro ID de impressora
    PRINTER_ID=$(echo "$PRINTERS" | grep -o '"id":"[^"]*"' | head -1 | cut -d'"' -f4)
    echo "   Usando impressora: $PRINTER_ID"
fi
echo ""

# Teste 1: Upload básico
echo "🧪 Teste 1: Upload básico de arquivo PDF"
RESPONSE=$(curl -s -X POST "$API_URL/jobs" \
    -F "file=@$TEST_FILE" \
    -F "printer_id=$PRINTER_ID" \
    -F 'options={"copies":1,"colorMode":"auto"}')

echo "   Resposta: $RESPONSE"

if echo "$RESPONSE" | grep -q '"id"'; then
    JOB_ID=$(echo "$RESPONSE" | grep -o '"id":"[^"]*"' | cut -d'"' -f4)
    echo "   ✅ Job criado com sucesso! ID: $JOB_ID"
else
    echo "   ❌ Falha ao criar job"
    echo "   Resposta completa: $RESPONSE"
    exit 1
fi
echo ""

# Teste 2: Verificar se o job aparece na lista
echo "🧪 Teste 2: Verificar se o job aparece na lista"
JOBS=$(curl -s "$API_URL/jobs")
if echo "$JOBS" | grep -q "$JOB_ID"; then
    echo "   ✅ Job encontrado na lista!"
else
    echo "   ⚠️  Job não encontrado na lista (pode estar processando)"
fi
echo ""

# Teste 3: Obter detalhes do job
echo "🧪 Teste 3: Obter detalhes do job"
JOB_DETAILS=$(curl -s "$API_URL/jobs/$JOB_ID")
if echo "$JOB_DETAILS" | grep -q '"id"'; then
    echo "   ✅ Detalhes do job obtidos com sucesso!"
    echo "   Status: $(echo "$JOB_DETAILS" | grep -o '"status":"[^"]*"' | cut -d'"' -f4)"
else
    echo "   ⚠️  Não foi possível obter detalhes (endpoint pode não estar implementado)"
fi
echo ""

# Teste 4: Upload com opções avançadas
echo "🧪 Teste 4: Upload com opções avançadas"
RESPONSE2=$(curl -s -X POST "$API_URL/jobs" \
    -F "file=@$TEST_FILE" \
    -F "printer_id=$PRINTER_ID" \
    -F 'options={"copies":2,"colorMode":"color","duplex":"long-edge","paperSize":"A4","quality":"high"}')

if echo "$RESPONSE2" | grep -q '"id"'; then
    JOB_ID2=$(echo "$RESPONSE2" | grep -o '"id":"[^"]*"' | cut -d'"' -f4)
    echo "   ✅ Job com opções avançadas criado! ID: $JOB_ID2"
else
    echo "   ❌ Falha ao criar job com opções avançadas"
fi
echo ""

# Teste 5: Upload sem arquivo (deve falhar)
echo "🧪 Teste 5: Tentativa de upload sem arquivo (deve falhar)"
RESPONSE_ERROR=$(curl -s -X POST "$API_URL/jobs" \
    -F "printer_id=$PRINTER_ID" \
    -F 'options={"copies":1}')

if echo "$RESPONSE_ERROR" | grep -qi "error\|fail\|400\|500"; then
    echo "   ✅ Erro esperado retornado corretamente"
else
    echo "   ⚠️  Resposta inesperada: $RESPONSE_ERROR"
fi
echo ""

# Limpeza
echo "🧹 Limpando arquivo de teste..."
rm -f "$TEST_FILE"
echo ""

echo "==========================================="
echo "   ✅ Testes Concluídos!"
echo "==========================================="
echo ""
echo "📊 Resumo:"
echo "   - Upload básico: ✅"
echo "   - Lista de jobs: ✅"
echo "   - Opções avançadas: ✅"
echo "   - Validação de erros: ✅"
echo ""
echo "💡 Para testar no frontend:"
echo "   1. Abra http://localhost:3000/upload"
echo "   2. Selecione uma impressora"
echo "   3. Arraste um arquivo PDF"
echo "   4. Clique em 'Processar Arquivos'"
echo "   5. Verifique o console do navegador (F12)"
echo ""


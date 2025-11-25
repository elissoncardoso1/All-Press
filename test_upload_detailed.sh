#!/bin/bash

# Teste detalhado de upload - All Press C++
# Testa o endpoint POST /api/jobs com diferentes cenários

set -e

API_URL="http://localhost:8000/api"
TEST_DIR="/tmp/all_press_test"
TEST_PDF="$TEST_DIR/test.pdf"
TEST_JPG="$TEST_DIR/test.jpg"

echo "==========================================="
echo "   Teste Detalhado de Upload"
echo "==========================================="
echo ""

# Criar diretório de teste
mkdir -p "$TEST_DIR"

# Criar arquivo PDF de teste
echo "📄 Criando arquivo PDF de teste..."
cat > "$TEST_PDF" << 'EOF'
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

# Criar arquivo JPG de teste (minimal)
echo "🖼️  Criando arquivo JPG de teste..."
# Criar um JPG mínimo válido
printf '\xff\xd8\xff\xe0\x00\x10JFIF\x00\x01\x01\x01\x00H\x00H\x00\x00\xff\xdb\x00C\x00\x08\x06\x06\x07\x06\x05\x08\x07\x07\x07\t\t\x08\n\x0c\x14\r\x0c\x0b\x0b\x0c\x19\x12\x13\x0f\x14\x1d\x1a\x1f\x1e\x1d\x1a\x1c\x1c $.\' ",#\x1c\x1c(7),01444\x1f\'9=82<.ff\xdb\x00C\x01\t\t\t\x0c\x0b\x0c\x18\r\r\x182!\x1c!22222222222222222222222222222222222222222222222222\xff\xc0\x00\x11\x08\x00\x01\x00\x01\x01\x01\x11\x00\x02\x11\x01\x03\x11\x01\xff\xc4\x00\x14\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\xff\xc4\x00\x14\x10\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xda\x00\x0c\x03\x01\x00\x02\x11\x03\x11\x00\x3f\x00\xaa\xff\xd9' > "$TEST_JPG"

echo "✅ Arquivos de teste criados"
echo ""

# Verificar se o servidor está rodando
echo "🔍 Verificando servidor..."
if ! curl -s "$API_URL/../" > /dev/null; then
    echo "❌ Servidor não está rodando!"
    exit 1
fi
echo "✅ Servidor está rodando"
echo ""

# Obter lista de impressoras
echo "🖨️  Obtendo impressoras..."
PRINTERS=$(curl -s "$API_URL/printers")
PRINTER_COUNT=$(echo "$PRINTERS" | grep -o '"id"' | wc -l | tr -d ' ')
echo "   Encontradas: $PRINTER_COUNT impressoras"

if [ "$PRINTER_COUNT" -eq 0 ]; then
    PRINTER_ID="default"
    echo "   ⚠️  Usando 'default' como ID"
else
    PRINTER_ID=$(echo "$PRINTERS" | grep -o '"id":"[^"]*"' | head -1 | cut -d'"' -f4)
    echo "   Usando: $PRINTER_ID"
fi
echo ""

# Teste 1: Upload PDF básico
echo "🧪 Teste 1: Upload PDF básico"
echo "   Arquivo: $TEST_PDF"
echo "   Tamanho: $(stat -f%z "$TEST_PDF" 2>/dev/null || stat -c%s "$TEST_PDF" 2>/dev/null) bytes"

RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "$API_URL/jobs" \
    -F "file=@$TEST_PDF" \
    -F "printer_id=$PRINTER_ID" \
    -F 'options={"copies":1,"colorMode":"auto"}')

HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE:" | cut -d: -f2)
BODY=$(echo "$RESPONSE" | sed '/HTTP_CODE:/d')

echo "   Status HTTP: $HTTP_CODE"
echo "   Resposta: $BODY"

if [ "$HTTP_CODE" = "200" ]; then
    if echo "$BODY" | grep -q '"id"'; then
        JOB_ID=$(echo "$BODY" | grep -o '"id":"[^"]*"' | cut -d'"' -f4)
        echo "   ✅ Sucesso! Job ID: $JOB_ID"
    else
        echo "   ⚠️  Resposta 200 mas sem ID de job"
    fi
else
    echo "   ❌ Falha! Código: $HTTP_CODE"
    echo "   Resposta completa: $BODY"
fi
echo ""

# Teste 2: Upload JPG
echo "🧪 Teste 2: Upload JPG"
RESPONSE2=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "$API_URL/jobs" \
    -F "file=@$TEST_JPG" \
    -F "printer_id=$PRINTER_ID" \
    -F 'options={"copies":1,"colorMode":"color"}')

HTTP_CODE2=$(echo "$RESPONSE2" | grep "HTTP_CODE:" | cut -d: -f2)
BODY2=$(echo "$RESPONSE2" | sed '/HTTP_CODE:/d')

echo "   Status HTTP: $HTTP_CODE2"
if [ "$HTTP_CODE2" = "200" ]; then
    echo "   ✅ Sucesso!"
else
    echo "   ⚠️  Status: $HTTP_CODE2"
fi
echo ""

# Teste 3: Upload sem arquivo (deve falhar)
echo "🧪 Teste 3: Upload sem arquivo (validação)"
RESPONSE3=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "$API_URL/jobs" \
    -F "printer_id=$PRINTER_ID" \
    -F 'options={"copies":1}')

HTTP_CODE3=$(echo "$RESPONSE3" | grep "HTTP_CODE:" | cut -d: -f2)
echo "   Status HTTP: $HTTP_CODE3"
if [ "$HTTP_CODE3" = "400" ] || [ "$HTTP_CODE3" = "500" ]; then
    echo "   ✅ Erro esperado retornado corretamente"
else
    echo "   ⚠️  Status inesperado: $HTTP_CODE3"
fi
echo ""

# Teste 4: Verificar jobs criados
echo "🧪 Teste 4: Verificar jobs na lista"
JOBS=$(curl -s "$API_URL/jobs")
JOB_COUNT=$(echo "$JOBS" | grep -o '"id"' | wc -l | tr -d ' ')
echo "   Jobs encontrados: $JOB_COUNT"
if [ "$JOB_COUNT" -gt 0 ]; then
    echo "   ✅ Jobs aparecem na lista"
else
    echo "   ⚠️  Nenhum job encontrado (pode estar processando)"
fi
echo ""

# Limpeza
echo "🧹 Limpando arquivos de teste..."
rm -rf "$TEST_DIR"
echo ""

echo "==========================================="
echo "   ✅ Testes Concluídos!"
echo "==========================================="
echo ""
echo "📋 Resumo:"
echo "   - Upload PDF: $([ "$HTTP_CODE" = "200" ] && echo "✅" || echo "❌")"
echo "   - Upload JPG: $([ "$HTTP_CODE2" = "200" ] && echo "✅" || echo "❌")"
echo "   - Validação: $([ "$HTTP_CODE3" = "400" ] || [ "$HTTP_CODE3" = "500" ] && echo "✅" || echo "⚠️")"
echo ""
echo "💡 Se algum teste falhou:"
echo "   1. Verifique os logs do backend: tail -f backend.log"
echo "   2. Verifique o console do navegador (F12)"
echo "   3. Teste manualmente em http://localhost:3000/upload"
echo ""


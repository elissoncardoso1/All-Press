#!/bin/bash

# All Press C++ - Script de Inicialização
# Inicia backend e frontend em processos separados

set -e

PROJECT_DIR="/Volumes/ElissonSSD2/Projetos/All_press_c"
BACKEND_DIR="$PROJECT_DIR/build"
FRONTEND_DIR="$PROJECT_DIR/frontend"

echo "==========================================="
echo "   All Press C++ - Iniciando Servidores"
echo "==========================================="
echo ""

# Verificar se o backend está compilado
if [ ! -f "$BACKEND_DIR/all_press_server" ]; then
    echo "❌ Backend não compilado. Compilando..."
    cd "$BACKEND_DIR"
    cmake .. && ninja
fi

# Matar processos existentes
echo "🧹 Limpando processos existentes..."
pkill -f all_press_server 2>/dev/null || true
pkill -f "vite" 2>/dev/null || true
sleep 2

# Liberar portas
echo "🔓 Liberando portas..."
lsof -ti:8000,8001,3000 2>/dev/null | xargs kill -9 2>/dev/null || true
sleep 1

# Iniciar backend
echo "🚀 Iniciando backend (porta 8000)..."
cd "$BACKEND_DIR"
./all_press_server > "$PROJECT_DIR/backend.log" 2>&1 &
BACKEND_PID=$!
echo "   Backend PID: $BACKEND_PID"
echo "   Log: $PROJECT_DIR/backend.log"

# Aguardar backend iniciar
sleep 3

# Verificar se backend está rodando
if ps -p $BACKEND_PID > /dev/null; then
    echo "✅ Backend iniciado com sucesso"
else
    echo "❌ Backend falhou ao iniciar. Verifique backend.log"
    exit 1
fi

# Iniciar frontend
echo "🚀 Iniciando frontend (porta 3000)..."
cd "$FRONTEND_DIR"
npm run dev > "$PROJECT_DIR/frontend.log" 2>&1 &
FRONTEND_PID=$!
echo "   Frontend PID: $FRONTEND_PID"
echo "   Log: $PROJECT_DIR/frontend.log"

# Aguardar frontend iniciar
sleep 5

# Verificar se frontend está rodando
if ps -p $FRONTEND_PID > /dev/null; then
    echo "✅ Frontend iniciado com sucesso"
else
    echo "❌ Frontend falhou ao iniciar. Verifique frontend.log"
    exit 1
fi

echo ""
echo "==========================================="
echo "   ✅ Servidores Iniciados!"
echo "==========================================="
echo ""
echo "📍 URLs:"
echo "   Backend API:  http://localhost:8000"
echo "   Frontend:     http://localhost:3000"
echo "   WebSocket:    ws://localhost:8001"
echo ""
echo "📋 Processos:"
echo "   Backend PID:  $BACKEND_PID"
echo "   Frontend PID: $FRONTEND_PID"
echo ""
echo "📝 Logs:"
echo "   Backend:      tail -f $PROJECT_DIR/backend.log"
echo "   Frontend:     tail -f $PROJECT_DIR/frontend.log"
echo ""
echo "🛑 Para parar os servidores:"
echo "   kill $BACKEND_PID $FRONTEND_PID"
echo ""


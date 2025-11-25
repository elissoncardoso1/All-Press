#!/bin/bash

# All Press C++ - Script para Parar Servidores

echo "🛑 Parando servidores All Press C++..."

# Matar processos
pkill -f all_press_server 2>/dev/null && echo "✅ Backend parado" || echo "⚠️  Backend não estava rodando"
pkill -f "vite" 2>/dev/null && echo "✅ Frontend parado" || echo "⚠️  Frontend não estava rodando"

# Liberar portas
lsof -ti:8000,8001,3000 2>/dev/null | xargs kill -9 2>/dev/null && echo "✅ Portas liberadas" || echo "⚠️  Nenhuma porta em uso"

echo ""
echo "✅ Todos os servidores foram parados"


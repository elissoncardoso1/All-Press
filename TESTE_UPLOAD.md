# 🧪 Guia de Teste - Upload de Arquivos

## Problemas Corrigidos

### ✅ 1. Sidebar sem item Upload
**Problema**: O componente `Sidebar.tsx` em `components/` não tinha os itens Upload e Monitor.

**Correção**: Adicionados os itens:
- Upload (ícone Upload)
- Monitor (ícone MonitorIcon)

### ✅ 2. Import incorreto no Layout
**Problema**: Layout.tsx estava importando Sidebar de `./Sidebar` em vez de `./layout/Sidebar`.

**Correção**: Corrigido o import para usar o Sidebar correto.

## Como Testar

### Teste 1: Verificar Sidebar
1. Abra http://localhost:3000
2. Verifique se a sidebar mostra:
   - ✅ Dashboard
   - ✅ Impressoras
   - ✅ Fila de Jobs
   - ✅ **Upload** (novo)
   - ✅ **Monitor** (novo)
   - ✅ Logs
   - ✅ Configurações

### Teste 2: Teste Manual no Frontend
1. Acesse http://localhost:3000/upload
2. Abra o console do navegador (F12)
3. Selecione uma impressora online
4. Arraste um arquivo PDF ou imagem
5. Configure as opções de impressão
6. Clique em "Processar Arquivos"
7. Verifique os logs no console:
   - `🔄 Iniciando processamento de arquivos...`
   - `📁 Arquivos: [...]`
   - `🖨️ Impressora selecionada: ...`
   - `📤 Enviando arquivo: ...`
   - `✅ Job criado: {...}` ou `❌ Erro...`

### Teste 3: Teste via Script Bash
```bash
# Execute o script de teste
./test_upload.sh
```

O script irá:
- ✅ Verificar se o servidor está rodando
- ✅ Obter lista de impressoras
- ✅ Criar job de teste
- ✅ Verificar se o job aparece na lista
- ✅ Testar upload com opções avançadas
- ✅ Testar validação de erros

### Teste 4: Teste via cURL
```bash
# Criar arquivo de teste
echo "%PDF-1.4" > /tmp/test.pdf

# Upload básico
curl -X POST http://localhost:8000/api/jobs \
  -F "file=@/tmp/test.pdf" \
  -F "printer_id=HP_LaserJet" \
  -F 'options={"copies":1,"colorMode":"auto"}'

# Verificar resposta (deve conter "id" e "status")
```

## Verificação de Problemas

### Se Upload não aparece na sidebar:
1. Verifique se o arquivo `frontend/src/components/Sidebar.tsx` tem o item Upload
2. Verifique o console do navegador por erros
3. Recarregue a página (Ctrl+R ou Cmd+R)
4. Limpe o cache do navegador

### Se arquivos não são processados:
1. **Verifique o console do navegador (F12)**:
   - Procure por erros em vermelho
   - Verifique os logs de debug que adicionamos
   - Veja a resposta da API

2. **Verifique os logs do backend**:
   ```bash
   tail -f backend.log
   # ou
   tail -f all_press.log
   ```

3. **Teste o endpoint diretamente**:
   ```bash
   curl -v -X POST http://localhost:8000/api/jobs \
     -F "file=@/tmp/test.pdf" \
     -F "printer_id=HP_LaserJet" \
     -F 'options={"copies":1}'
   ```

4. **Verifique se a impressora está online**:
   ```bash
   curl http://localhost:8000/api/printers | python3 -m json.tool
   ```

## Logs Esperados

### Console do Navegador (Sucesso):
```
🔄 Iniciando processamento de arquivos...
📁 Arquivos: [{id: "...", file: File, status: "ready", ...}]
🖨️ Impressora selecionada: HP_LaserJet
⚙️ Opções: {copies: 1, colorMode: "auto", ...}
📤 Enviando arquivo: document.pdf
✅ Job criado: {id: "123", status: "pending", ...}
```

### Console do Navegador (Erro):
```
🔄 Iniciando processamento de arquivos...
📁 Arquivos: [...]
🖨️ Impressora selecionada: HP_LaserJet
📤 Enviando arquivo: document.pdf
❌ Erro ao processar document.pdf: Error: ...
```

### Backend Log (Sucesso):
```
[INFO] Handling POST /api/jobs
[INFO] Job created successfully: ID=123
```

### Backend Log (Erro):
```
[ERROR] Failed to create job: ...
```

## Checklist de Verificação

- [ ] Sidebar mostra item "Upload"
- [ ] Sidebar mostra item "Monitor"
- [ ] Página /upload carrega corretamente
- [ ] Lista de impressoras aparece
- [ ] É possível selecionar uma impressora
- [ ] É possível fazer upload de arquivo (drag & drop)
- [ ] Botão "Processar Arquivos" está habilitado quando há arquivo
- [ ] Console mostra logs de debug
- [ ] Job é criado com sucesso
- [ ] Toast de sucesso aparece
- [ ] Redirecionamento para /jobs funciona após sucesso

## Próximos Passos se Ainda Houver Problemas

1. Verifique a versão do backend (deve ser 1.1.0)
2. Verifique se o backend foi recompilado após as mudanças
3. Verifique se há erros de CORS no console
4. Teste com diferentes tipos de arquivo (PDF, JPG, PNG)
5. Verifique se o tamanho do arquivo não excede limites

---

**Última Atualização**: 2025-01-XX
**Versão**: 1.1.0


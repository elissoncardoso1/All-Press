# Teste de Depuração de Impressão - All Press C++

## 📋 Visão Geral

Este diretório contém ferramentas completas de depuração e teste para o sistema de impressão All Press C++. **Todos os testes usam dados reais do sistema, sem mocks ou simulações.**

## 🛠️ Ferramentas Disponíveis

### 1. Interface Web de Teste (`print_debug_test.html`)

Interface completa e visual para testes de impressão diretamente no navegador.

**Características:**
- ✅ **Status em tempo real** do sistema
- ✅ **Lista de impressoras** detectadas com status real
- ✅ **Upload de arquivos** com suporte a todos os 16 formatos
- ✅ **Teste de impressão** com seleção de impressora e cópias
- ✅ **Logs detalhados** com timestamps e colorização
- ✅ **Progress indicators** para uploads e impressão
- ✅ **Exportação de logs** para análise

**Como usar:**
```bash
# Abra o arquivo no navegador após iniciar o backend
open frontend/print_debug_test.html
# ou
# http://localhost:3001/print_debug_test.html (com frontend rodando)
```

### 2. Script de Linha de Comando (`debug_print_test.js`)

Script Node.js para testes automatizados e debugging via terminal.

**Comandos disponíveis:**

```bash
# Verificar status do sistema
node debug_print_test.js status

# Listar impressoras
node debug_print_test.js printers

# Listar formatos suportados
node debug_print_test.js formats

# Testar impressão de arquivo específico
node debug_print_test.js test /caminho/do/arquivo.pdf

# Executar teste completo automatizado
node debug_print_test.js full
```

**Características:**
- ✅ **Conexão real** com a API REST (porta 8000)
- ✅ **Upload real** de arquivos para processamento
- ✅ **Monitoramento real** de status de jobs
- ✅ **Criação automática** de arquivos de teste
- ✅ **Logs coloridos** no terminal
- ✅ **Tratamento de erros** completo

### 3. Arquivos de Teste

Arquivos exemplo para testes de impressão:

- `test_files/sample_document.txt` - Documento de texto simples
- `test_files/simple_test.html` - Documento HTML com formatação completa
- **Arquivos criados automaticamente** pelo script quando necessário

## 🖨️ Impressoras Detectadas (Reais)

O sistema detecta automaticamente as seguintes impressoras via CUPS:

1. **Canon G3010 Series** (USB)
   - Status: Offline
   - URI: `usb://Canon/G3010%20series?serial=A5DEDD`

2. **Canon G3010 Series (WiFi)** ⭐
   - Status: Online
   - URI: `dnssd://Canon%20G3010%20series._ipps._tcp.local./`
   - **Recomendada para testes**

3. **EPSON L395 Series** (Network)
   - Status: Offline
   - URI: `dnssd://EPSON%20L395%20Series._pdl-datastream._tcp.local./`

4. **EPSON L4260 Series** (Network)
   - Status: Offline
   - URI: `dnssd://EPSON%20L4260%20Series._ipps._tcp.local./`

5. **Kyocera ECOSYS M3040idn** (Network)
   - Status: Offline
   - URI: `dnssd://Kyocera%20ECOSYS%20M3040idn%20(2)._ipps._tcp.local./`

## 📄 Formatos Suportados (16 formatos)

### Documentos Office
- ✅ **PDF** - Portable Document Format
- ✅ **DOCX** - Microsoft Word 2007+
- ✅ **DOC** - Microsoft Word 97-2003
- ✅ **XLSX** - Microsoft Excel 2007+
- ✅ **XLS** - Microsoft Excel 97-2003
- ✅ **PPTX** - Microsoft PowerPoint 2007+
- ✅ **PPT** - Microsoft PowerPoint 97-2003

### CAD e Design
- ✅ **DWG** - AutoCAD Drawing
- ✅ **DXF** - AutoCAD DXF
- ✅ **SVG** - Scalable Vector Graphics
- ✅ **AI** - Adobe Illustrator
- ✅ **PSD** - Adobe Photoshop
- ✅ **CDR** - CorelDRAW
- ✅ **EPS** - Encapsulated PostScript

### Imagens
- ✅ **JPG/JPEG** - JPEG Image
- ✅ **PNG** - Portable Network Graphics

## 🚀 Fluxo de Teste Completo

### 1. Preparação
```bash
# Iniciar o backend (se não estiver rodando)
cd /Volumes/ElissonSSD2/Projetos/All_press_c
./build/all_press_server

# Verificar se está rodando na porta 8000
curl http://localhost:8000/api/status
```

### 2. Teste via Interface Web
1. Abra `frontend/print_debug_test.html` no navegador
2. Verifique o status do sistema
3. Confirme as impressoras detectadas
4. Arraste ou selecione um arquivo para teste
5. Selecione uma impressora online
6. Clique em "Iniciar Teste de Impressão"
7. Monitore os logs em tempo real

### 3. Teste via Linha de Comando
```bash
# Teste completo automatizado
node debug_print_test.js full

# Ou teste manual de arquivo específico
node debug_print_test.js test test_files/simple_test.html
```

## 📊 API Endpoints Utilizados

Os testes usam os seguintes endpoints reais da API:

- `GET /api/status` - Status do sistema
- `GET /api/printers` - Lista de impressoras
- `GET /api/system/info` - Informações e formatos suportados
- `POST /api/jobs` - Criar job de impressão
- `GET /api/jobs/{id}` - Status de job específico

## 🔍 Logs e Depuração

### Logs da Interface Web
- ✅ **Timestamps** precisos
- ✅ **Coloração** por tipo (info, success, warning, error)
- ✅ **Exportação** para arquivo `.txt`
- ✅ **Auto-scroll** para logs mais recentes

### Logs do Script CLI
- ✅ **Saída colorida** no terminal
- ✅ **Progress indicators** para operações longas
- ✅ **Detalhes completos** de requisições HTTP
- ✅ **Error stack traces** quando aplicável

### Logs do Backend
O backend gera logs detalhados para:
- ✅ **Status de impressoras** (verificação a cada 5 segundos)
- ✅ **Processamento de arquivos** (conversão, validação)
- ✅ **Status de jobs** (queued, processing, completed, failed)
- ✅ **Erros de sistema** (conexão, CUPS, conversão)

## 🐛 Troubleshooting Comum

### Backend não responde
```bash
# Verificar se está rodando
lsof -i :8000

# Reiniciar se necessário
pkill -f all_press_server
./build/all_press_server
```

### Impressora offline
1. Verifique conexão física (USB, rede)
2. Confirme impressora ligada
3. Verifique configuração CUPS: `http://localhost:631`

### Formato não suportado
1. Verifique extensão do arquivo (maiúsculas/minúsculas)
2. Confirme se está na lista dos 16 formatos suportados
3. Verifique se o arquivo não está corrompido

### Job de impressão falha
1. Verifique logs do backend para erro específico
2. Confirme se impressora está online
3. Verifique espaço em disco para conversão
4. Teste com arquivo simples (TXT, HTML)

## 📈 Métricas e Estatísticas

A interface exibe em tempo real:
- **Printers Detectadas**: Total de impressoras CUPS
- **Jobs na Fila**: Número de jobs pendentes
- **Jobs Ativos**: Jobs em processamento
- **Formatos Suportados**: Total de 16 formatos

## 🔄 Automação

O script `debug_print_test.js` pode ser integrado em:
- **CI/CD pipelines** para testes automatizados
- **Scripts de monitoramento** para verificação de saúde
- **Systemd services** para testes periódicos
- **Cron jobs** para testes agendados

## 📝 Relatórios

Após cada teste, você pode:
- ✅ **Exportar logs** completos
- ✅ **Gerar relatórios** de status
- ✅ **Arquivar resultados** para análise histórica
- ✅ **Comparar desempenho** entre testes

---

**Nota**: Todos os testes foram projetados para usar **dados reais do sistema** e **sem mocks**, garantindo resultados fiáveis e representativos do comportamento real do All Press C++ em produção.
# Teste de Status de Impressoras

## Problema
O indicador de status (círculos verde/vermelho) não está mostrando os dados reais das impressoras.

## Solução Implementada

### 1. Backend - Verificação Real do Status (C++)
**Arquivo**: `src/core/printer_manager.cpp`

Implementada verificação real do status usando IPP (Internet Printing Protocol) via CUPS:

```cpp
// Verificar status real da impressora usando CUPS
ipp_t* request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
std::string printer_uri = "ipp://localhost/printers/" + std::string(dest->name);
ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, printer_uri.c_str());

ipp_t* response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");

// Por padrão, considerar offline
info.is_online = false;
info.status = 5; // stopped

if (response) {
    ipp_attribute_t* attr = ippFindAttribute(response, "printer-state", IPP_TAG_ENUM);
    if (attr) {
        int state = ippGetInteger(attr, 0);
        info.status = state;
        // IPP_PRINTER_IDLE (3) = online e pronta
        // IPP_PRINTER_PROCESSING (4) = online e imprimindo
        // IPP_PRINTER_STOPPED (5) = parada/offline
        info.is_online = (state == 3 || state == 4);
    }
    
    // Verificar razões de estado (erros, sem papel, etc)
    attr = ippFindAttribute(response, "printer-state-reasons", IPP_TAG_KEYWORD);
    if (attr) {
        for (int j = 0; j < ippGetCount(attr); j++) {
            const char* reason = ippGetString(attr, j, nullptr);
            if (reason) {
                std::string reason_str(reason);
                if (reason_str.find("offline") != std::string::npos ||
                    reason_str.find("shutdown") != std::string::npos ||
                    reason_str.find("connecting-to-device") != std::string::npos) {
                    info.is_online = false;
                }
            }
        }
    }
    
    ippDelete(response);
}
```

### 2. Estados IPP
Os estados das impressoras CUPS seguem o padrão IPP:
- **Estado 3 (IPP_PRINTER_IDLE)**: Impressora online e pronta
- **Estado 4 (IPP_PRINTER_PROCESSING)**: Impressora online e processando trabalho
- **Estado 5 (IPP_PRINTER_STOPPED)**: Impressora parada/offline

### 3. Logs Adicionados

**Backend**: Agora mostra no console o status de cada impressora descoberta:
```
Printer: HP_LaserJet - Status: 3 - Online: YES
Printer: Canon_Plotter - Status: 5 - Online: NO
```

**Frontend**: Console do navegador mostra os dados recebidos:
```
📡 Printers received from API: [...]
  🖨️  HP_LaserJet: status=online, online=true
  🖨️  Canon_Plotter: status=offline, online=false
```

## Como Testar

### 1. Iniciar o Backend
```bash
cd /Volumes/ElissonSSD2/Projetos/All_press_c/build
./all_press_server
```

Observe os logs no terminal. Você verá algo como:
```
[INFO] Printer: NOME_IMPRESSORA - Status: X - Online: YES/NO
```

### 2. Iniciar o Frontend
```bash
cd /Volumes/ElissonSSD2/Projetos/All_press_c/frontend
npm run dev
```

### 3. Abrir o Console do Navegador
1. Abra o navegador em `http://localhost:5173`
2. Abra as Developer Tools (F12)
3. Vá na aba "Console"
4. Navegue até a página de Upload

Você verá:
```
📡 Printers received from API: [...]
  🖨️  NOME_IMPRESSORA: status=online/offline, online=true/false
```

### 4. Verificar Visual
Na página de Upload:
- **🟢 Círculo Verde**: Impressora online
- **🔴 Círculo Vermelho**: Impressora offline
- Impressoras offline estarão **desabilitadas** (não clicáveis)

## Verificação Manual do Status via CUPS

Para verificar manualmente o status de uma impressora:

```bash
# Listar impressoras
lpstat -p

# Ver status detalhado de uma impressora
lpstat -p NOME_IMPRESSORA -l

# Ver todas as impressoras com detalhes
lpstat -t
```

Exemplo de saída:
```
printer HP_LaserJet is idle. enabled since Mon Nov 17 10:00:00 2025
printer Canon_Plotter disabled since Mon Nov 17 09:00:00 2025
```

## Testando Estados Diferentes

### Simular Impressora Offline
```bash
# Desabilitar impressora (ficará offline)
cupsdisable NOME_IMPRESSORA

# Recarregar página no navegador
# A impressora deve aparecer com 🔴
```

### Reativar Impressora
```bash
# Habilitar impressora (ficará online)
cupsenable NOME_IMPRESSORA

# Recarregar página no navegador
# A impressora deve aparecer com 🟢
```

## Fluxo Completo de Dados

```
Backend (C++)
├── discover_cups_printers()
│   └── Para cada impressora:
│       ├── ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES)
│       ├── ippFindAttribute("printer-state")
│       └── Define is_online baseado no estado
│
└── handle_get_printers()
    └── Retorna JSON: {"status": "online"} ou {"status": "offline"}
    
↓

Frontend (TypeScript)
├── printerAPI.getAll()
│   └── GET /api/printers
│
├── printerStore.fetchPrinters()
│   └── Salva em state.printers
│
└── Upload.tsx
    └── Renderiza indicadores:
        - printer.status === 'online' → 🟢
        - printer.status !== 'online' → 🔴
```

## Troubleshooting

### Todas as impressoras aparecem offline
1. Verificar se o CUPS está rodando:
   ```bash
   sudo systemctl status cups  # Linux
   sudo launchctl list | grep cups  # macOS
   ```

2. Verificar se há impressoras configuradas:
   ```bash
   lpstat -p
   ```

3. Verificar logs do backend no terminal

### Impressoras não aparecem
1. Verificar se a API está respondendo:
   ```bash
   curl http://localhost:8001/api/printers
   ```

2. Verificar console do navegador por erros

### Status não atualiza
1. Recarregar a página (F5)
2. O backend busca status atual a cada chamada de `/api/printers`
3. Para atualização em tempo real, usar WebSocket (já implementado)

## Próximos Passos

Se o problema persistir:
1. ✅ Verificar logs do backend
2. ✅ Verificar console do navegador
3. ✅ Testar endpoint diretamente com curl
4. ✅ Verificar estado real das impressoras com lpstat


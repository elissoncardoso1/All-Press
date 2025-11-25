# Verificação Real de Conectividade de Impressoras

## Problema Original
As impressoras apareciam como "online" mesmo quando o computador não estava conectado à rede, pois o sistema apenas consultava o cache local do CUPS.

## Solução Implementada

### Verificação em Duas Etapas

#### 1️⃣ **Primeira Etapa: Verificar CUPS Local**
```cpp
// Consulta o estado local que o CUPS tem da impressora
ipp_t* request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
ipp_t* response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");

// Verifica se o CUPS diz que está pronta (estado 3 ou 4)
bool cups_says_ready = (state == 3 || state == 4);
```

#### 2️⃣ **Segunda Etapa: Verificar Conectividade de Rede REAL**
```cpp
// SE o CUPS diz que está pronta, testar conexão TCP real
if (cups_says_ready) {
    // Extrair host do URI (ex: ipp://192.168.1.100:631)
    // Tentar conectar com timeout de 2 segundos
    http_t* http_conn = httpConnect2(host, port, ..., 2000, ...);
    
    if (http_conn) {
        info.is_online = true;  // ✅ Conectou = REALMENTE ONLINE
        httpClose(http_conn);
    } else {
        info.is_online = false; // ❌ Falhou = OFFLINE NA REDE
    }
}
```

### Lógica de Decisão

```
┌─────────────────────────────────────────────────────────────┐
│                     DESCOBRIR IMPRESSORA                    │
└─────────────────────────────────────────────────────────────┘
                            │
                            ↓
        ┌───────────────────────────────────────┐
        │  Obter URI real (device-uri)          │
        │  Ex: ipp://192.168.1.100:631          │
        │      socket://192.168.1.101:9100      │
        │      usb://Brother/HL-L2350DW         │
        └───────────────────────────────────────┘
                            │
                            ↓
        ┌───────────────────────────────────────┐
        │  Consultar CUPS local                 │
        │  Está em estado 3 ou 4?               │
        └───────────────────────────────────────┘
                   │                    │
              NÃO  │                    │  SIM
                   ↓                    ↓
        ┌──────────────────┐   ┌──────────────────┐
        │  is_online=FALSE │   │  É impressora    │
        │  status=5        │   │  de REDE?        │
        │  (OFFLINE)       │   └──────────────────┘
        └──────────────────┘            │
                              ┌─────────┴─────────┐
                          NÃO │                   │ SIM
                              ↓                   ↓
                   ┌──────────────────┐  ┌──────────────────┐
                   │  USB/Local       │  │  Tentar conectar │
                   │  Confiar no CUPS │  │  TCP com timeout │
                   │  is_online=TRUE  │  │  de 2 segundos   │
                   └──────────────────┘  └──────────────────┘
                                                  │
                                         ┌────────┴────────┐
                                         │                 │
                                    SUCESSO            FALHA
                                         │                 │
                                         ↓                 ↓
                              ┌──────────────────┐  ┌──────────────────┐
                              │  is_online=TRUE  │  │  is_online=FALSE │
                              │  ✅ REALMENTE    │  │  ❌ SEM REDE     │
                              │     ONLINE       │  │                  │
                              └──────────────────┘  └──────────────────┘
```

## Tipos de Impressoras Detectadas

### 🌐 Impressoras de Rede
**URIs**: `ipp://`, `http://`, `socket://`, `lpd://`
**Verificação**: Tenta conectar TCP no host especificado
**Timeout**: 2 segundos

Exemplos:
- `ipp://192.168.1.100:631/ipp/print`
- `socket://192.168.1.101:9100`
- `lpd://printer.local/queue`

### 🔌 Impressoras Locais (USB)
**URIs**: `usb://`, `serial://`, `parallel://`
**Verificação**: Confia no status do CUPS
**Razão**: Não há "rede" para testar, o CUPS sabe se está conectada

Exemplos:
- `usb://Brother/HL-L2350DW?serial=XXXX`
- `serial://dev/ttyUSB0`

### 💾 Impressoras Virtuais
**URIs**: `file://`, `cups-pdf://`
**Verificação**: Confia no CUPS (sempre online se configurada)

Exemplos:
- `cups-pdf:/`
- `file:///tmp/output.pdf`

## Logs Detalhados

Agora o sistema mostra logs completos para debug:

```
[INFO] Printer: HP_LaserJet_Pro 
       | URI: ipp://192.168.1.100:631/ipp/print
       | CUPS Status: 3 
       | CUPS Ready: YES
       | Network Online: NO
       
[INFO] Printer: Brother_USB 
       | URI: usb://Brother/HL-L2350DW?serial=E7J123456
       | CUPS Status: 3 
       | CUPS Ready: YES
       | Network Online: YES
       
[INFO] Printer: Canon_Offline 
       | URI: ipp://192.168.1.200:631
       | CUPS Status: 5 
       | CUPS Ready: NO
       | Network Online: NO
```

### Interpretando os Logs

| CUPS Ready | Network Online | Resultado Final | Explicação |
|-----------|----------------|-----------------|------------|
| YES | YES | 🟢 **ONLINE** | Impressora de rede acessível |
| YES | NO | 🔴 **OFFLINE** | CUPS OK, mas sem conectividade |
| NO | - | 🔴 **OFFLINE** | CUPS já diz que está offline |
| YES (USB) | YES | 🟢 **ONLINE** | Impressora USB conectada |

## Cenários de Teste

### ✅ Cenário 1: Computador Sem Rede
```bash
# Desconectar Wi-Fi/Ethernet
# Executar servidor
./all_press_server

# Resultado esperado:
# Impressoras de rede: 🔴 OFFLINE (não consegue conectar)
# Impressoras USB: 🟢 ONLINE (conectadas diretamente)
```

### ✅ Cenário 2: Impressora de Rede Desligada
```bash
# Impressora configurada mas desligada/desconectada
# Executar servidor
./all_press_server

# Resultado esperado:
# A impressora específica: 🔴 OFFLINE (timeout na conexão)
# Outras impressoras: status normal
```

### ✅ Cenário 3: Todas Conectadas
```bash
# Wi-Fi conectado, impressoras ligadas
./all_press_server

# Resultado esperado:
# Todas impressoras de rede: 🟢 ONLINE
# Impressoras USB conectadas: 🟢 ONLINE
```

## Timeout e Performance

### ⏱️ Timeout de Conexão: 2 segundos por impressora

**Por que 2 segundos?**
- Rápido o suficiente para não travar a UI
- Longo o suficiente para redes lentas
- Timeout padrão de 30s seria muito lento

**Exemplo de cálculo:**
- 1 impressora de rede: ~2 segundos
- 3 impressoras de rede: ~6 segundos
- 5 impressoras de rede: ~10 segundos

### 🚀 Otimização Futura (não implementado ainda)
- Verificação paralela (threads)
- Cache de status (atualizar a cada 30s)
- Verificação apenas de impressoras que mudaram

## Testando Manualmente

### Ver URIs das Impressoras
```bash
lpstat -v
```

Saída exemplo:
```
device for HP_LaserJet: ipp://192.168.1.100:631/ipp/print
device for Brother_USB: usb://Brother/HL-L2350DW?serial=E7J123456
device for Canon_Plotter: socket://192.168.1.150:9100
```

### Testar Conectividade Manual
```bash
# Para impressoras IPP
curl -v --connect-timeout 2 http://192.168.1.100:631

# Para impressoras socket (raw)
nc -zv -w 2 192.168.1.150 9100

# Para impressoras LPD
telnet 192.168.1.160 515
```

### Simular Falha de Rede
```bash
# Bloquear IP específico (macOS/Linux)
sudo route add -host 192.168.1.100 127.0.0.1

# Executar servidor
./all_press_server

# Impressora deve aparecer OFFLINE

# Restaurar
sudo route delete 192.168.1.100
```

## Diferença Entre Estado CUPS e Estado Real

### CUPS diz "IDLE" (Estado 3)
**O que significa**: "Na última vez que verifiquei, estava pronta"
**Problema**: Pode estar desatualizado se a impressora foi desligada

### Nossa Verificação
**O que faz**: "Vou tentar conectar AGORA"
**Resultado**: Status em tempo real, baseado em conectividade TCP

## Exemplo de Fluxo Completo

```
1. Usuário abre página de Upload
2. Frontend chama GET /api/printers
3. Backend executa discover_cups_printers()
4. Para cada impressora:
   a. CUPS diz: "HP_Printer está IDLE (3)"
   b. URI: ipp://192.168.1.100:631
   c. Tenta conectar TCP em 192.168.1.100:631
   d. Timeout após 2 segundos (SEM REDE)
   e. Marca: is_online = FALSE
5. Backend retorna JSON: {"status": "offline"}
6. Frontend mostra: 🔴 HP_Printer (desabilitado)
```

## Mensagens de Erro Comuns

### "Printer X - Network Online: NO" mas deveria estar online
**Causas possíveis**:
- Firewall bloqueando porta 631 ou 9100
- Impressora em subnet diferente
- VPN/proxy interferindo
- Timeout muito curto (2s)

**Solução**:
```bash
# Testar conectividade manual
ping 192.168.1.100
telnet 192.168.1.100 631
```

### Todas impressoras aparecem offline
**Causas**:
- Sem rede
- CUPS não rodando: `sudo systemctl start cups`
- Nenhuma impressora configurada: `lpstat -p`

## Resumo

✅ **O que foi melhorado**:
- Verificação real de conectividade TCP
- Timeout de 2 segundos (não trava)
- Distinção entre impressoras de rede e locais
- Logs detalhados para debug
- Status preciso refletindo conectividade real

❌ **O que NÃO foi mudado**:
- Descoberta de impressoras (ainda usa CUPS)
- Configuração de impressoras
- Envio de jobs de impressão

🎯 **Resultado final**:
Agora, se você não estiver conectado à rede, as impressoras de rede aparecerão como **OFFLINE (🔴)** enquanto impressoras USB locais continuarão **ONLINE (🟢)**.


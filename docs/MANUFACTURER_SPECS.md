# Especificações dos Fabricantes

## HP DesignJet Series

### Documentação Oficial
- **Site de Suporte**: https://www.hp.com/en/support.html
- **Downloads de Drivers**: https://support.hp.com/drivers
- **Referência Técnica**: https://www.hp.com/en/designjet/technical-reference

### DesignJet T1200 (2015)

**Especificações:**
- Largura máxima: 1118mm (44")
- Protocolos: HPGL2, PostScript, PDF
- Resoluções: 300, 600, 1200 DPI
- Sistema de tinta: HP 72 (6 cores)

**Quirks:**
- Delay de alimentação de papel: 500ms
- Requer aquecimento das canetas
- Calibração de cor periódica recomendada

### DesignJet T2300 (2018)

**Especificações:**
- Largura máxima: 1118mm (44")
- Protocolos: HPGL2 (primário), PostScript, PDF
- Resoluções: 300, 600, 1200 DPI
- Sistema de tinta: HP 72 (6 cores)

**Quirks:**
- Delay de alimentação: 300ms
- Calibração de cor obrigatória
- Melhor performance com HPGL2

### DesignJet T3500 (2020)

**Especificações:**
- Largura máxima: 1118mm (44")
- Protocolos: HPGL2, PostScript, PDF
- Resoluções: 300, 600, 1200 DPI
- Sistema de tinta: HP 72 (6 cores)
- Modo alta velocidade disponível
- Touchscreen integrado

**Quirks:**
- Delay de alimentação: 200ms
- Suporta modo alta velocidade
- Alimentador automático de folhas

## Canon imagePROGRAF Series

### Documentação Oficial
- **Site de Suporte**: https://www.canon.com/en/business/
- **Downloads**: https://www.canon.com/support
- **Documentação Técnica**: https://www.canon.com/imageprograf/technical

### imagePROGRAF TX-3000 (2019)

**Especificações:**
- Largura máxima: 914mm (36")
- Comprimento máximo: 18m (roll)
- Protocolos: PostScript (primário), PDF, HPGL2
- Resoluções: 300, 600, 1200, 2400 DPI
- Sistema de tinta: LUCIA TD Pigment (5 cores)

**Características:**
- Suporte a perfil ICC obrigatório
- Gerenciamento de cor avançado
- Hot swap de cartuchos de tinta

**Quirks:**
- Perfil ICC requerido para melhor qualidade
- Suporte UltraChrome ink

### imagePROGRAF TX-4000 (2020)

**Especificações:**
- Largura máxima: 1118mm (44")
- Comprimento máximo: 18m (roll)
- Protocolos: PostScript (primário), PDF, HPGL2
- Resoluções: 300, 600, 1200, 2400 DPI
- Sistema de tinta: LUCIA PRO Pigment (11 cores)

**Características:**
- LUCIA PRO ink system
- Sensor de densidade de cor
- Precisão de cor profissional

**Quirks:**
- LUCIA PRO ink para qualidade fotográfica
- Perfil ICC requerido

### imagePROGRAF PRO-6000 (2021)

**Especificações:**
- Largura máxima: 1524mm (60")
- Comprimento máximo: 18m (roll)
- Protocolos: PostScript, PDF
- Resoluções: 600, 1200, 2400 DPI
- Sistema de tinta: LUCIA PRO 12-Color

**Características:**
- 12 cores para gamut expandido
- Nível profissional
- Color matching avançado
- Precisão extrema

**Quirks:**
- Requer 12 cartuchos de tinta
- Configuração profissional recomendada

## Epson SureColor Series

### Documentação Oficial
- **Site de Suporte**: https://epson.com/en/support
- **Downloads**: https://epson.com/drivers
- **Guia Técnico**: https://epson.com/surecolor/technical

### SureColor T5200 (2018)

**Especificações:**
- Largura máxima: 914mm (36")
- Comprimento máximo: 15m (roll)
- Protocolos: PostScript (primário), ESC/P, PDF
- Resoluções: 300, 600, 720, 1200, 2400 DPI
- Sistema de tinta: UltraChrome XD2 Pigment (4 cores)

**Características:**
- Modo SureColor
- Auto take-up reel
- PrecisionCore technology

**Quirks:**
- Largura máxima de roll: 1118mm
- Modo SureColor ativado

### SureColor T7200 (2019)

**Especificações:**
- Largura máxima: 1118mm (44")
- Comprimento máxima: 15m (roll)
- Protocolos: PostScript, ESC/P, PDF
- Resoluções: 300, 600, 720, 1200, 2400 DPI
- Sistema de tinta: UltraChrome XD2 (4 cores)

**Quirks:**
- UltraChrome XD2 para cores vibrantes
- Largura máxima: 1118mm

### SureColor T7700 (2021)

**Especificações:**
- Largura máxima: 1118mm (44")
- Comprimento máximo: 15m (roll)
- Protocolos: PostScript, ESC/P, PDF
- Resoluções: 300, 600, 720, 1200, 2400 DPI
- Sistema de tinta: UltraChrome XD3 (6 cores)

**Características:**
- Suporte a dual roll
- Auto switching entre rolls
- PrecisionCore TFP printhead
- Modo de alta velocidade

**Quirks:**
- Dual roll support
- UltraChrome XD3 system

## Comparativo de Protocolos

### HPGL/HPGL2
- **Vantagens**: Otimizado para desenhos vetoriais, menor tamanho de arquivo
- **Desvantagens**: Requer pré-processamento, complexo para raster
- **Melhor uso**: CAD, desenhos técnicos

### PostScript
- **Vantagens**: Universal, não requer pré-processamento, excelente para imagens
- **Desvantagens**: Arquivos maiores
- **Melhor uso**: Documentos mistos, fotos, gráficos

### ESC/P
- **Vantagens**: Simples, rápido para texto
- **Desvantagens**: Limitado para gráficos complexos
- **Melhor uso**: Texto, gráficos simples

## Sistemas de Tinta

### HP 72 (6 cores)
- Photo Black
- Matte Black
- Cyan
- Magenta
- Yellow
- Gray

### Canon LUCIA TD (5 cores)
- Pigment Black
- Cyan
- Magenta
- Yellow
- Matte Black

### Canon LUCIA PRO (11 cores)
- Photo Black
- Matte Black
- Cyan
- Magenta
- Yellow
- Photo Cyan
- Photo Magenta
- Gray
- Photo Gray
- Chroma Optimizer
- Matte Black

### Epson UltraChrome XD2 (4 cores)
- Cyan
- Magenta
- Yellow
- Black

### Epson UltraChrome XD3 (6 cores)
- Cyan
- Magenta
- Yellow
- Black
- Light Cyan
- Light Magenta

## Tamanhos de Papel Suportados

| Tamanho | Dimensões (mm) | HP | Canon | Epson |
|---------|----------------|-----|-------|-------|
| A0 | 841 x 1189 | ✓ | ✓ | ✓ |
| A1 | 594 x 841 | ✓ | ✓ | ✓ |
| A2 | 420 x 594 | ✓ | ✓ | ✓ |
| A3 | 297 x 420 | ✓ | ✓ | ✓ |
| A4 | 210 x 297 | ✓ | ✓ | ✓ |
| Letter | 216 x 279 | ✓ | ✓ | ✓ |
| Legal | 216 x 356 | ✓ | ✓ | ✓ |
| Tabloid | 279 x 432 | ✓ | ✓ | ✓ |
| Roll | Custom | ✓ | ✓ | ✓ |

## Resoluções Máximas

| Fabricante | Modelo | DPI Máximo | Recomendado |
|------------|--------|------------|-------------|
| HP | T1200 | 1200 | 600 |
| HP | T2300 | 1200 | 600 |
| HP | T3500 | 1200 | 600 |
| Canon | TX-3000 | 2400 | 600 |
| Canon | TX-4000 | 2400 | 1200 |
| Canon | PRO-6000 | 2400 | 1200 |
| Epson | T5200 | 2400 | 720 |
| Epson | T7200 | 2400 | 720 |
| Epson | T7700 | 2400 | 720 |


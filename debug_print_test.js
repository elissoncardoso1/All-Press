#!/usr/bin/env node

/**
 * All Press C++ - Print Debugging Test Script
 * Teste de depuração de impressão sem mocks - dados reais do sistema
 */

const fs = require('fs');
const path = require('path');
const http = require('http');
const https = require('https');
const { spawn } = require('child_process');

const API_BASE = 'http://localhost:8000';
const WS_BASE = 'ws://localhost:8001';

// Cores para console
const colors = {
    reset: '\x1b[0m',
    bright: '\x1b[1m',
    red: '\x1b[31m',
    green: '\x1b[32m',
    yellow: '\x1b[33m',
    blue: '\x1b[34m',
    magenta: '\x1b[35m',
    cyan: '\x1b[36m',
    white: '\x1b[37m'
};

// Logger colorido
function log(message, color = 'white') {
    const timestamp = new Date().toLocaleString('pt-BR');
    console.log(`${colors[color]}[${timestamp}] ${message}${colors.reset}`);
}

// Fazer requisição HTTP
function makeRequest(method, url, data = null) {
    return new Promise((resolve, reject) => {
        const lib = url.startsWith('https') ? https : http;
        const urlObj = new URL(url);

        const options = {
            hostname: urlObj.hostname,
            port: urlObj.port,
            path: urlObj.pathname + urlObj.search,
            method: method,
            headers: {
                'Content-Type': 'application/json',
                'User-Agent': 'All-Press-Debug-Script/1.0'
            }
        };

        if (data) {
            const jsonData = JSON.stringify(data);
            options.headers['Content-Length'] = Buffer.byteLength(jsonData);
        }

        const req = lib.request(options, (res) => {
            let body = '';
            res.on('data', chunk => body += chunk);
            res.on('end', () => {
                try {
                    resolve({
                        statusCode: res.statusCode,
                        headers: res.headers,
                        data: body ? JSON.parse(body) : null
                    });
                } catch (e) {
                    resolve({
                        statusCode: res.statusCode,
                        headers: res.headers,
                        data: body
                    });
                }
            });
        });

        req.on('error', reject);
        req.setTimeout(10000, () => reject(new Error('Timeout')));

        if (data) {
            req.write(JSON.stringify(data));
        }
        req.end();
    });
}

// Fazer upload de arquivo
function uploadFile(filePath, printerId, copies = 1) {
    return new Promise((resolve, reject) => {
        if (!fs.existsSync(filePath)) {
            return reject(new Error(`Arquivo não encontrado: ${filePath}`));
        }

        const fileStats = fs.statSync(filePath);
        const boundary = '----WebKitFormBoundary' + Math.random().toString(36).substring(2);
        const formData = [];

        // Adicionar campos do formulário
        formData.push(
            `--${boundary}`,
            'Content-Disposition: form-data; name="printerId"',
            '',
            printerId,
            `--${boundary}`,
            'Content-Disposition: form-data; name="copies"',
            '',
            copies.toString(),
            `--${boundary}`,
            'Content-Disposition: form-data; name="priority"',
            '',
            'normal',
            `--${boundary}`,
            'Content-Disposition: form-data; name="title"',
            '',
            `Debug Test: ${path.basename(filePath)}`,
            `--${boundary}`,
            `Content-Disposition: form-data; name="file"; filename="${path.basename(filePath)}"`,
            `Content-Type: ${getContentType(filePath)}`,
            '',
            fs.readFileSync(filePath),
            `--${boundary}--`
        );

        const formDataLength = formData.reduce((sum, part) => sum + (Buffer.isBuffer(part) ? part.length : Buffer.byteLength(part)), 0);

        const options = {
            hostname: 'localhost',
            port: 8000,
            path: '/api/jobs',
            method: 'POST',
            headers: {
                'Content-Type': `multipart/form-data; boundary=${boundary}`,
                'Content-Length': formDataLength,
                'User-Agent': 'All-Press-Debug-Script/1.0'
            }
        };

        const req = http.request(options, (res) => {
            let body = '';
            res.on('data', chunk => body += chunk);
            res.on('end', () => {
                try {
                    resolve({
                        statusCode: res.statusCode,
                        data: body ? JSON.parse(body) : null
                    });
                } catch (e) {
                    resolve({
                        statusCode: res.statusCode,
                        data: body
                    });
                }
            });
        });

        req.on('error', reject);
        req.setTimeout(30000, () => reject(new Error('Upload timeout')));

        // Enviar formData
        formData.forEach(part => {
            if (Buffer.isBuffer(part)) {
                req.write(part);
            } else {
                req.write(part);
            }
        });

        req.end();
    });
}

// Obter content-type do arquivo
function getContentType(filePath) {
    const ext = path.extname(filePath).toLowerCase();
    const types = {
        '.pdf': 'application/pdf',
        '.doc': 'application/msword',
        '.docx': 'application/vnd.openxmlformats-officedocument.wordprocessingml.document',
        '.xls': 'application/vnd.ms-excel',
        '.xlsx': 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
        '.ppt': 'application/vnd.ms-powerpoint',
        '.pptx': 'application/vnd.openxmlformats-officedocument.presentationml.presentation',
        '.jpg': 'image/jpeg',
        '.jpeg': 'image/jpeg',
        '.png': 'image/png',
        '.svg': 'image/svg+xml',
        '.dwg': 'application/acad',
        '.dxf': 'application/dxf',
        '.ai': 'application/illustrator',
        '.psd': 'application/photoshop',
        '.cdr': 'application/coreldraw',
        '.eps': 'application/postscript'
    };
    return types[ext] || 'application/octet-stream';
}

// Verificar status do sistema
async function checkSystemStatus() {
    log('🔍 Verificando status do sistema...', 'cyan');

    try {
        const response = await makeRequest('GET', `${API_BASE}/api/system/status`);

        if (response.statusCode === 200) {
            log('✅ Conexão com backend estabelecida', 'green');
            log(`📊 Status da API: ${response.statusCode}`, 'white');
            log(`🖥️ Sistema: ${response.data.status} | Versão: ${response.data.version}`, 'white');
            log(`📊 CUPS: ${response.data.cupsConnected ? 'Conectado' : 'Desconectado'} | Database: ${response.data.databaseConnected ? 'Conectado' : 'Desconectado'}`, 'white');
            return response.data;
        } else {
            log(`❌ Erro na API: ${response.statusCode}`, 'red');
            return null;
        }
    } catch (error) {
        log(`❌ Falha na conexão: ${error.message}`, 'red');
        return null;
    }
}

// Listar impressoras
async function listPrinters() {
    log('🖨️ Listando impressoras disponíveis...', 'cyan');

    try {
        const response = await makeRequest('GET', `${API_BASE}/api/printers`);

        if (response.statusCode === 200 && response.data) {
            const printers = Array.isArray(response.data) ? response.data : [];

            if (printers.length === 0) {
                log('⚠️ Nenhuma impressora encontrada', 'yellow');
                return [];
            }

            log(`📊 Encontradas ${printers.length} impressoras:`, 'white');

            printers.forEach((printer, index) => {
                const status = printer.isOnline ? '🟢 ONLINE' : '🔴 OFFLINE';
                log(`  ${index + 1}. ${printer.name} (${printer.id}) - ${status}`, 'white');
                log(`     URI: ${printer.uri || 'N/A'}`, 'white');
                log(`     CUPS Status: ${printer.cupsStatus || 'N/A'} | Ready: ${printer.cupsReady ? 'Sim' : 'Não'}`, 'white');
                log('', 'white');
            });

            return printers;
        } else {
            log(`❌ Erro ao listar impressoras: ${response.statusCode}`, 'red');
            return [];
        }
    } catch (error) {
        log(`❌ Falha ao listar impressoras: ${error.message}`, 'red');
        return [];
    }
}

// Listar formatos suportados
async function listFormats() {
    log('📄 Verificando formatos suportados...', 'cyan');

    try {
        // Obter formatos da primeira impressora (todas suportam os mesmos formatos)
        const printersResponse = await makeRequest('GET', `${API_BASE}/api/printers`);

        if (printersResponse.statusCode === 200 && printersResponse.data && printersResponse.data.length > 0) {
            const formats = printersResponse.data[0].capabilities.supportedFormats || [];

            log(`📊 ${formats.length} formatos suportados:`, 'white');

            const formatGroups = {
                'Documentos': ['pdf', 'docx', 'doc', 'xlsx', 'xls', 'pptx', 'ppt'],
                'Design/CAD': ['dwg', 'dxf', 'svg', 'ai', 'psd', 'cdr', 'eps'],
                'Imagens': ['jpg', 'jpeg', 'png']
            };

            Object.entries(formatGroups).forEach(([category, formatList]) => {
                log(`  ${category}:`, 'bright');
                const supportedInCategory = formatList.filter(f => formats.includes(f));
                if (supportedInCategory.length > 0) {
                    supportedInCategory.forEach(format => {
                        log(`    ✅ .${format.toUpperCase()}`, 'green');
                    });
                } else {
                    log(`    ❌ Nenhum formato desta categoria`, 'red');
                }
            });

            return formats;
        } else {
            log(`❌ Erro ao obter formatos: ${printersResponse.statusCode}`, 'red');
            return [];
        }
    } catch (error) {
        log(`❌ Falha ao obter formatos: ${error.message}`, 'red');
        return [];
    }
}

// Criar arquivo de teste
function createTestFile(format) {
    const testDir = path.join(__dirname, 'test_files');
    if (!fs.existsSync(testDir)) {
        fs.mkdirSync(testDir, { recursive: true });
    }

    let filePath;
    let content;

    switch (format.toLowerCase()) {
        case 'txt':
            filePath = path.join(testDir, `test_${Date.now()}.txt`);
            content = `Arquivo de Teste - All Press C++\nData: ${new Date().toISOString()}\n\nEste é um arquivo de teste para o sistema de impressão.\n\nConteúdo de exemplo:\n- Linha 1\n- Linha 2\n- Linha 3\n\nFim do teste.`;
            fs.writeFileSync(filePath, content, 'utf8');
            break;

        case 'html':
            filePath = path.join(testDir, `test_${Date.now()}.html`);
            content = `<!DOCTYPE html>
<html>
<head>
    <title>Teste de Impressão - All Press C++</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .header { text-align: center; border-bottom: 2px solid #333; padding-bottom: 20px; }
        .content { margin: 20px 0; line-height: 1.6; }
        .footer { text-align: center; margin-top: 40px; font-size: 12px; color: #666; }
    </style>
</head>
<body>
    <div class="header">
        <h1>🖨️ Teste de Impressão</h1>
        <h2>All Press C++ - Sistema de Gerenciamento</h2>
    </div>

    <div class="content">
        <h3>Informações do Teste:</h3>
        <p><strong>Data:</strong> ${new Date().toLocaleString('pt-BR')}</p>
        <p><strong>Sistema:</strong> All Press C++ v1.0.0</p>
        <p><strong>Tipo:</strong> Teste de Depuração</p>

        <h3>Características Testadas:</h3>
        <ul>
            <li>✅ Renderização de texto</li>
            <li>✅ Formatação CSS</li>
            <li>✅ Layout responsivo</li>
            <li>✅ Integração com backend</li>
        </ul>

        <h3>Resultado Esperado:</h3>
        <p>Este documento deve ser impresso corretamente mantendo formatação, cores e layout.</p>
    </div>

    <div class="footer">
        <p>Gerado pelo script de debug - All Press C++</p>
        <p>Página 1 de 1</p>
    </div>
</body>
</html>`;
            fs.writeFileSync(filePath, content, 'utf8');
            break;

        default:
            // Criar arquivo de texto genérico
            filePath = path.join(testDir, `test_${Date.now()}.${format}`);
            content = `Test File - All Press C++\nDate: ${new Date().toISOString()}\nFormat: ${format}\n\nThis is a test file for printing system debugging.\n\nContent:\n1. Test line 1\n2. Test line 2\n3. Test line 3\n\nEnd of test.`;
            fs.writeFileSync(filePath, content, 'utf8');
    }

    return filePath;
}

// Testar impressão
async function testPrint(filePath, printerId, copies = 1) {
    log(`🖨️ Iniciando teste de impressão...`, 'cyan');
    log(`📁 Arquivo: ${path.basename(filePath)}`, 'white');
    log(`🖨️ Impressora: ${printerId}`, 'white');
    log(`📄 Cópias: ${copies}`, 'white');

    try {
        const response = await uploadFile(filePath, printerId, copies);

        if (response.statusCode === 201 || response.statusCode === 200) {
            log('✅ Job de impressão criado com sucesso!', 'green');
            log(`📊 ID do Job: ${response.data.id || 'N/A'}`, 'white');
            log(`📊 Status: ${response.data.status || 'N/A'}`, 'white');
            log(`📊 Prioridade: ${response.data.priority || 'N/A'}`, 'white');

            // Monitorar status do job
            if (response.data.id) {
                await monitorJob(response.data.id);
            }

            return response.data;
        } else {
            log(`❌ Erro ao criar job: ${response.statusCode}`, 'red');
            if (response.data) {
                log(`Detalhes: ${JSON.stringify(response.data)}`, 'red');
            }
            return null;
        }
    } catch (error) {
        log(`❌ Erro no teste de impressão: ${error.message}`, 'red');
        return null;
    }
}

// Monitorar job
async function monitorJob(jobId, maxAttempts = 30) {
    log(`📡 Monitorando job ${jobId}...`, 'cyan');

    for (let i = 0; i < maxAttempts; i++) {
        try {
            const response = await makeRequest('GET', `${API_BASE}/api/jobs/${jobId}`);

            if (response.statusCode === 200 && response.data) {
                const job = response.data;
                const status = job.status || 'unknown';

                let statusIcon;
                switch (status.toLowerCase()) {
                    case 'completed': statusIcon = '✅'; break;
                    case 'failed': statusIcon = '❌'; break;
                    case 'processing': statusIcon = '⚡'; break;
                    case 'queued': statusIcon = '⏳'; break;
                    default: statusIcon = '📋';
                }

                log(`  ${statusIcon} Job ${jobId}: ${status.toUpperCase()} (${job.progress || 0}%)`, 'white');

                if (status.toLowerCase() === 'completed') {
                    log(`🎉 Job ${jobId} concluído com sucesso!`, 'green');
                    return job;
                } else if (status.toLowerCase() === 'failed') {
                    log(`💥 Job ${jobId} falhou! Erro: ${job.error || 'Desconhecido'}`, 'red');
                    return job;
                }
            } else {
                log(`⚠️ Erro ao verificar status do job: ${response.statusCode}`, 'yellow');
            }
        } catch (error) {
            log(`⚠️ Erro ao monitorar job: ${error.message}`, 'yellow');
        }

        // Esperar 2 segundos antes da próxima verificação
        await new Promise(resolve => setTimeout(resolve, 2000));
    }

    log(`⏰ Timeout ao monitorar job ${jobId}`, 'yellow');
    return null;
}

// Teste completo automatizado
async function runFullTest() {
    log('🚀 Iniciando teste completo de depuração...', 'bright');
    log('=' .repeat(60), 'cyan');

    // 1. Verificar status do sistema
    const systemStatus = await checkSystemStatus();
    if (!systemStatus) {
        log('❌ Não foi possível conectar ao sistema. Verifique se o backend está rodando.', 'red');
        return;
    }

    log('', 'white');

    // 2. Listar impressoras
    const printers = await listPrinters();
    const onlinePrinters = printers.filter(p => p.isOnline);

    if (onlinePrinters.length === 0) {
        log('⚠️ Nenhuma impressora online encontrada. Teste não pode prosseguir.', 'yellow');
        return;
    }

    log('', 'white');

    // 3. Listar formatos
    const formats = await listFormats();

    log('', 'white');

    // 4. Criar arquivo de teste
    const testFile = createTestFile('html');
    log(`📝 Arquivo de teste criado: ${testFile}`, 'green');
    log(`📊 Tamanho: ${fs.statSync(testFile).size} bytes`, 'white');

    log('', 'white');

    // 5. Realizar teste de impressão
    const targetPrinter = onlinePrinters[0]; // Usar primeira impressora online
    log(`🎯 Alvo do teste: ${targetPrinter.name} (${targetPrinter.id})`, 'cyan');

    const result = await testPrint(testFile, targetPrinter.id, 1);

    log('', 'white');
    log('=' .repeat(60), 'cyan');

    if (result) {
        log('🎉 Teste completo finalizado com sucesso!', 'green');
    } else {
        log('❌ Teste completo falhou!', 'red');
    }

    // Limpar arquivo de teste
    try {
        fs.unlinkSync(testFile);
        log(`🗑️ Arquivo de teste removido: ${testFile}`, 'white');
    } catch (error) {
        log(`⚠️ Não foi possível remover arquivo de teste: ${error.message}`, 'yellow');
    }
}

// Função principal
async function main() {
    const args = process.argv.slice(2);

    log('🖨️ All Press C++ - Print Debugging Script', 'bright');
    log('Script de teste de depuração de impressão (sem mocks)', 'white');
    log('', 'white');

    if (args.length === 0) {
        log('Uso:', 'cyan');
        log('  node debug_print_test.js [comando] [opções]', 'white');
        log('', 'white');
        log('Comandos disponíveis:', 'cyan');
        log('  status          - Verificar status do sistema', 'white');
        log('  printers        - Listar impressoras', 'white');
        log('  formats         - Listar formatos suportados', 'white');
        log('  test <arquivo>  - Testar impressão de arquivo específico', 'white');
        log('  full            - Executar teste completo automatizado', 'white');
        log('', 'white');
        log('Exemplos:', 'cyan');
        log('  node debug_print_test.js status', 'white');
        log('  node debug_print_test.js test /path/to/file.pdf', 'white');
        log('  node debug_print_test.js full', 'white');
        return;
    }

    const command = args[0].toLowerCase();

    switch (command) {
        case 'status':
            await checkSystemStatus();
            break;

        case 'printers':
            await listPrinters();
            break;

        case 'formats':
            await listFormats();
            break;

        case 'test':
            if (args.length < 2) {
                log('❌ É necessário especificar o caminho do arquivo para teste', 'red');
                return;
            }

            const printers = await listPrinters();
            const onlinePrinters = printers.filter(p => p.isOnline);

            if (onlinePrinters.length === 0) {
                log('❌ Nenhuma impressora online encontrada', 'red');
                return;
            }

            await testPrint(args[1], onlinePrinters[0].id, 1);
            break;

        case 'full':
            await runFullTest();
            break;

        default:
            log(`❌ Comando desconhecido: ${command}`, 'red');
            break;
    }
}

// Tratar exceções não capturadas
process.on('unhandledRejection', (reason, promise) => {
    log(`❌ Unhandled Rejection: ${reason}`, 'red');
    process.exit(1);
});

process.on('uncaughtException', (error) => {
    log(`❌ Uncaught Exception: ${error.message}`, 'red');
    log(error.stack, 'red');
    process.exit(1);
});

// Executar função principal
main().catch(error => {
    log(`❌ Erro na execução principal: ${error.message}`, 'red');
    process.exit(1);
});
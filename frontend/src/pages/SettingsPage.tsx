import React, { useState, useEffect } from 'react';
import { SettingsSection, SettingItem } from '@/components/SettingsSection';
import { Save, RotateCcw, Loader2 } from 'lucide-react';
import toast from 'react-hot-toast';
import { systemAPI } from '@/services/api';

interface Settings {
    // Server Settings
    httpPort: number;
    websocketPort: number;
    maxWorkers: number;

    // Printer Settings
    autoDiscovery: boolean;
    discoveryInterval: number;
    defaultPrinter: string;
    defaultPaperSize: string;
    defaultQuality: string;

    // Color Management
    colorProfile: string;
    enableColorManagement: boolean;

    // Database
    databasePath: string;

    // Logging
    logLevel: string;
    logFile: string;
}

const defaultSettings: Settings = {
    httpPort: 8000,
    websocketPort: 8001,
    maxWorkers: 4,
    autoDiscovery: true,
    discoveryInterval: 300,
    defaultPrinter: 'auto',
    defaultPaperSize: 'A4',
    defaultQuality: 'normal',
    colorProfile: 'sRGB',
    enableColorManagement: true,
    databasePath: './all_press.db',
    logLevel: 'INFO',
    logFile: './all_press.log',
};

export const SettingsPage: React.FC = () => {
    const [settings, setSettings] = useState<Settings>(defaultSettings);
    const [hasChanges, setHasChanges] = useState(false);
    const [isLoading, setIsLoading] = useState(false);

    // Load settings from API on mount
    useEffect(() => {
        const loadSettings = async () => {
            try {
                const serverSettings = await systemAPI.getSettings();
                
                // Map server settings to local settings format
                const mappedSettings: Settings = {
                    httpPort: serverSettings.server?.port || defaultSettings.httpPort,
                    websocketPort: serverSettings.server?.ws_port || defaultSettings.websocketPort,
                    maxWorkers: serverSettings.queue?.max_workers || defaultSettings.maxWorkers,
                    autoDiscovery: serverSettings.printer?.auto_discover !== undefined 
                        ? serverSettings.printer.auto_discover 
                        : defaultSettings.autoDiscovery,
                    discoveryInterval: serverSettings.printer?.discover_interval || defaultSettings.discoveryInterval,
                    defaultPrinter: defaultSettings.defaultPrinter,
                    defaultPaperSize: defaultSettings.defaultPaperSize,
                    defaultQuality: defaultSettings.defaultQuality,
                    colorProfile: serverSettings.color?.default_input_profile || defaultSettings.colorProfile,
                    enableColorManagement: serverSettings.color?.enable_calibration !== undefined
                        ? serverSettings.color.enable_calibration
                        : defaultSettings.enableColorManagement,
                    databasePath: serverSettings.database?.path || defaultSettings.databasePath,
                    logLevel: serverSettings.logging?.level || defaultSettings.logLevel,
                    logFile: serverSettings.logging?.file || defaultSettings.logFile,
                };
                
                setSettings(mappedSettings);
            } catch (error) {
                console.error('Error loading settings from server:', error);
                // Fallback to localStorage if API fails
                const savedSettings = localStorage.getItem('all_press_settings');
                if (savedSettings) {
                    try {
                        const parsed = JSON.parse(savedSettings);
                        setSettings({ ...defaultSettings, ...parsed });
                    } catch (e) {
                        console.error('Error loading settings from localStorage:', e);
                    }
                }
            }
        };
        
        loadSettings();
    }, []);

    const handleChange = <K extends keyof Settings>(key: K, value: Settings[K]) => {
        setSettings(prev => ({ ...prev, [key]: value }));
        setHasChanges(true);
    };

    const handleSave = async () => {
        setIsLoading(true);
        try {
            // Map local settings to server format
            const serverSettings = {
                server: {
                    port: settings.httpPort,
                    ws_port: settings.websocketPort,
                },
                queue: {
                    max_workers: settings.maxWorkers,
                },
                printer: {
                    auto_discover: settings.autoDiscovery,
                    discover_interval: settings.discoveryInterval,
                },
                color: {
                    default_input_profile: settings.colorProfile,
                    enable_calibration: settings.enableColorManagement,
                },
                database: {
                    path: settings.databasePath,
                },
                logging: {
                    level: settings.logLevel,
                    file: settings.logFile,
                },
            };
            
            // Save to server
            await systemAPI.saveSettings(serverSettings);
            
            // Also save to localStorage as backup
            localStorage.setItem('all_press_settings', JSON.stringify(settings));
            
            toast.success('Configurações salvas com sucesso!');
            setHasChanges(false);
        } catch (error) {
            console.error('Error saving settings:', error);
            let errorMessage = 'Erro desconhecido';
            if (error instanceof Error) {
                errorMessage = error.message;
            } else if (typeof error === 'object' && error !== null && 'response' in error) {
                const axiosError = error as { response?: { data?: { error?: string } }; message?: string };
                errorMessage = axiosError.response?.data?.error || axiosError.message || errorMessage;
            }
            toast.error(`Erro ao salvar configurações: ${errorMessage}`);
        } finally {
            setIsLoading(false);
        }
    };

    const handleReset = () => {
        setSettings(defaultSettings);
        setHasChanges(true);
        toast.success('Configurações restauradas para os valores padrão');
    };

    return (
        <div className="space-y-6">
            {/* Header */}
            <div className="flex items-center justify-between">
                <div>
                    <h2 className="text-3xl font-bold text-foreground tracking-tight">Configurações</h2>
                    <p className="text-muted-foreground mt-1">Preferências do sistema</p>
                </div>
                <div className="flex gap-2">
                    <button
                        onClick={handleReset}
                        className="flex items-center gap-2 px-4 py-2 bg-card text-foreground border border-border rounded-lg hover:bg-muted transition-colors"
                    >
                        <RotateCcw className="w-4 h-4" />
                        Restaurar Padrões
                    </button>
                    <button
                        onClick={handleSave}
                        disabled={!hasChanges || isLoading}
                        className="flex items-center gap-2 px-4 py-2 bg-primary text-primary-foreground rounded-lg hover:bg-primary/90 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                    >
                        {isLoading ? (
                            <>
                                <Loader2 className="w-4 h-4 animate-spin" />
                                Salvando...
                            </>
                        ) : (
                            <>
                                <Save className="w-4 h-4" />
                                Salvar Alterações
                            </>
                        )}
                    </button>
                </div>
            </div>

            {/* Server Settings */}
            <SettingsSection
                title="Configurações do Servidor"
                description="Configurações de rede e performance do servidor"
            >
                <SettingItem
                    label="Porta HTTP"
                    description="Porta para a API REST"
                >
                    <input
                        type="number"
                        value={settings.httpPort}
                        onChange={(e) => handleChange('httpPort', parseInt(e.target.value))}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                    />
                </SettingItem>

                <SettingItem
                    label="Porta WebSocket"
                    description="Porta para atualizações em tempo real"
                >
                    <input
                        type="number"
                        value={settings.websocketPort}
                        onChange={(e) => handleChange('websocketPort', parseInt(e.target.value))}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                    />
                </SettingItem>

                <SettingItem
                    label="Workers Máximos"
                    description="Número de threads para processamento paralelo"
                >
                    <input
                        type="number"
                        min="1"
                        max="16"
                        value={settings.maxWorkers}
                        onChange={(e) => handleChange('maxWorkers', parseInt(e.target.value))}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                    />
                </SettingItem>
            </SettingsSection>

            {/* Printer Settings */}
            <SettingsSection
                title="Configurações de Impressora"
                description="Configurações padrão para impressão"
            >
                <SettingItem
                    label="Descoberta Automática"
                    description="Buscar impressoras automaticamente na rede"
                >
                    <label className="relative inline-flex items-center cursor-pointer">
                        <input
                            type="checkbox"
                            checked={settings.autoDiscovery}
                            onChange={(e) => handleChange('autoDiscovery', e.target.checked)}
                            className="sr-only peer"
                        />
                        <div className="w-11 h-6 bg-gray-200 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-primary/20 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-primary"></div>
                    </label>
                </SettingItem>

                <SettingItem
                    label="Intervalo de Descoberta"
                    description="Intervalo em segundos para buscar novas impressoras"
                >
                    <input
                        type="number"
                        min="60"
                        max="3600"
                        value={settings.discoveryInterval}
                        onChange={(e) => handleChange('discoveryInterval', parseInt(e.target.value))}
                        disabled={!settings.autoDiscovery}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary disabled:opacity-50"
                    />
                </SettingItem>

                <SettingItem
                    label="Impressora Padrão"
                    description="Impressora selecionada por padrão"
                >
                    <select
                        value={settings.defaultPrinter}
                        onChange={(e) => handleChange('defaultPrinter', e.target.value)}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                    >
                        <option value="auto">Automático</option>
                        <option value="first">Primeira Disponível</option>
                    </select>
                </SettingItem>

                <SettingItem
                    label="Tamanho de Papel Padrão"
                    description="Tamanho de papel padrão para impressão"
                >
                    <select
                        value={settings.defaultPaperSize}
                        onChange={(e) => handleChange('defaultPaperSize', e.target.value)}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                    >
                        <option value="A4">A4</option>
                        <option value="Letter">Letter</option>
                        <option value="Legal">Legal</option>
                        <option value="A3">A3</option>
                    </select>
                </SettingItem>

                <SettingItem
                    label="Qualidade Padrão"
                    description="Qualidade de impressão padrão"
                >
                    <select
                        value={settings.defaultQuality}
                        onChange={(e) => handleChange('defaultQuality', e.target.value)}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                    >
                        <option value="draft">Rascunho</option>
                        <option value="normal">Normal</option>
                        <option value="high">Alta</option>
                    </select>
                </SettingItem>
            </SettingsSection>

            {/* Color Management */}
            <SettingsSection
                title="Gerenciamento de Cores"
                description="Configurações de perfis de cor e ICC"
            >
                <SettingItem
                    label="Gerenciamento de Cores"
                    description="Ativar gerenciamento de cores ICC"
                >
                    <label className="relative inline-flex items-center cursor-pointer">
                        <input
                            type="checkbox"
                            checked={settings.enableColorManagement}
                            onChange={(e) => handleChange('enableColorManagement', e.target.checked)}
                            className="sr-only peer"
                        />
                        <div className="w-11 h-6 bg-gray-200 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-primary/20 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-primary"></div>
                    </label>
                </SettingItem>

                <SettingItem
                    label="Perfil de Cor"
                    description="Perfil ICC para gerenciamento de cores"
                >
                    <select
                        value={settings.colorProfile}
                        onChange={(e) => handleChange('colorProfile', e.target.value)}
                        disabled={!settings.enableColorManagement}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary disabled:opacity-50"
                    >
                        <option value="sRGB">sRGB</option>
                        <option value="AdobeRGB">Adobe RGB</option>
                        <option value="ProPhoto">ProPhoto RGB</option>
                    </select>
                </SettingItem>
            </SettingsSection>

            {/* Database Settings */}
            <SettingsSection
                title="Banco de Dados"
                description="Configurações do banco de dados SQLite"
            >
                <SettingItem
                    label="Caminho do Banco"
                    description="Localização do arquivo de banco de dados"
                >
                    <input
                        type="text"
                        value={settings.databasePath}
                        onChange={(e) => handleChange('databasePath', e.target.value)}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary font-mono text-sm"
                    />
                </SettingItem>
            </SettingsSection>

            {/* Logging Settings */}
            <SettingsSection
                title="Logs"
                description="Configurações de registro de eventos"
            >
                <SettingItem
                    label="Nível de Log"
                    description="Nível mínimo de mensagens a serem registradas"
                >
                    <select
                        value={settings.logLevel}
                        onChange={(e) => handleChange('logLevel', e.target.value)}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                    >
                        <option value="DEBUG">Debug</option>
                        <option value="INFO">Info</option>
                        <option value="WARNING">Warning</option>
                        <option value="ERROR">Error</option>
                    </select>
                </SettingItem>

                <SettingItem
                    label="Arquivo de Log"
                    description="Localização do arquivo de log"
                >
                    <input
                        type="text"
                        value={settings.logFile}
                        onChange={(e) => handleChange('logFile', e.target.value)}
                        className="w-full px-3 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary font-mono text-sm"
                    />
                </SettingItem>
            </SettingsSection>

            {/* Save reminder */}
            {hasChanges && (
                <div className="bg-yellow-50 border border-yellow-200 rounded-lg p-4 flex items-center justify-between">
                    <p className="text-sm text-yellow-800">
                        Você tem alterações não salvas
                    </p>
                    <button
                        onClick={handleSave}
                        className="px-4 py-2 bg-yellow-600 text-white rounded-lg hover:bg-yellow-700 transition-colors text-sm font-medium"
                    >
                        Salvar Agora
                    </button>
                </div>
            )}
        </div>
    );
};

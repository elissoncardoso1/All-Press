import React, { useState, useMemo } from 'react';
import { useLogs, type LogEntry } from '@/hooks/useLogs';
import { LogEntryComponent } from '@/components/LogEntry';
import { Search, Filter, RefreshCw, Download, Loader2, ToggleLeft, ToggleRight } from 'lucide-react';

export const LogsPage: React.FC = () => {
    const [autoRefresh, setAutoRefresh] = useState(false);
    const { logs, loading, error, fetchLogs } = useLogs(autoRefresh, 5000);
    const [searchTerm, setSearchTerm] = useState('');
    const [levelFilter, setLevelFilter] = useState<LogEntry['level'] | 'all'>('all');

    const filteredLogs = useMemo(() => {
        return logs.filter(log => {
            const matchesSearch = log.message.toLowerCase().includes(searchTerm.toLowerCase());
            const matchesLevel = levelFilter === 'all' || log.level === levelFilter;
            return matchesSearch && matchesLevel;
        });
    }, [logs, searchTerm, levelFilter]);

    const stats = useMemo(() => {
        return {
            total: logs.length,
            info: logs.filter(l => l.level === 'INFO').length,
            warning: logs.filter(l => l.level === 'WARNING').length,
            error: logs.filter(l => l.level === 'ERROR').length,
            debug: logs.filter(l => l.level === 'DEBUG').length,
        };
    }, [logs]);

    const handleExport = () => {
        const logText = filteredLogs
            .map(log => {
                const timestamp = typeof log.timestamp === 'string' ? log.timestamp : log.timestamp.toISOString();
                return `[${timestamp}] [${log.level}] ${log.message}`;
            })
            .join('\n');

        const blob = new Blob([logText], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `all_press_logs_${new Date().toISOString()}.txt`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    };

    return (
        <div className="space-y-6">
            {/* Header */}
            <div className="flex items-center justify-between">
                <div>
                    <h2 className="text-3xl font-bold text-foreground tracking-tight">Logs do Sistema</h2>
                    <p className="text-muted-foreground mt-1">Histórico de eventos e erros</p>
                </div>
                <div className="flex items-center gap-2">
                    <button
                        onClick={() => setAutoRefresh(!autoRefresh)}
                        className={`flex items-center gap-2 px-4 py-2 rounded-lg border transition-colors ${autoRefresh
                            ? 'bg-primary text-primary-foreground border-primary'
                            : 'bg-card text-foreground border-border hover:bg-muted'
                            }`}
                    >
                        {autoRefresh ? <ToggleRight className="w-4 h-4" /> : <ToggleLeft className="w-4 h-4" />}
                        Auto-refresh
                    </button>
                    <button
                        onClick={() => fetchLogs()}
                        disabled={loading}
                        className="flex items-center gap-2 px-4 py-2 bg-primary text-primary-foreground rounded-lg hover:bg-primary/90 transition-colors disabled:opacity-50"
                    >
                        <RefreshCw className={`w-4 h-4 ${loading ? 'animate-spin' : ''}`} />
                        Atualizar
                    </button>
                    <button
                        onClick={handleExport}
                        disabled={filteredLogs.length === 0}
                        className="flex items-center gap-2 px-4 py-2 bg-card text-foreground border border-border rounded-lg hover:bg-muted transition-colors disabled:opacity-50"
                    >
                        <Download className="w-4 h-4" />
                        Exportar
                    </button>
                </div>
            </div>

            {/* Stats Cards */}
            <div className="grid grid-cols-2 md:grid-cols-5 gap-4">
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Total</div>
                    <div className="text-2xl font-bold text-foreground">{stats.total}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Info</div>
                    <div className="text-2xl font-bold text-blue-600">{stats.info}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Warnings</div>
                    <div className="text-2xl font-bold text-yellow-600">{stats.warning}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Errors</div>
                    <div className="text-2xl font-bold text-red-600">{stats.error}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Debug</div>
                    <div className="text-2xl font-bold text-gray-600">{stats.debug}</div>
                </div>
            </div>

            {/* Filters */}
            <div className="bg-card border border-border rounded-lg p-4">
                <div className="flex flex-col md:flex-row gap-4">
                    {/* Search */}
                    <div className="flex-1 relative">
                        <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-muted-foreground" />
                        <input
                            type="text"
                            placeholder="Buscar nos logs..."
                            value={searchTerm}
                            onChange={(e) => setSearchTerm(e.target.value)}
                            className="w-full pl-10 pr-4 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                        />
                    </div>

                    {/* Level Filter */}
                    <div className="flex items-center gap-2">
                        <Filter className="w-4 h-4 text-muted-foreground" />
                        <select
                            value={levelFilter}
                            onChange={(e) => setLevelFilter(e.target.value as LogEntry['level'] | 'all')}
                            className="px-4 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                        >
                            <option value="all">Todos os Níveis</option>
                            <option value="INFO">Info</option>
                            <option value="WARNING">Warning</option>
                            <option value="ERROR">Error</option>
                            <option value="DEBUG">Debug</option>
                        </select>
                    </div>
                </div>
            </div>

            {/* Logs List */}
            <div className="bg-card border border-border rounded-lg p-4">
                <div className="space-y-2 max-h-[600px] overflow-y-auto">
                    {loading && logs.length === 0 ? (
                        <div className="flex items-center justify-center py-12">
                            <Loader2 className="w-8 h-8 text-primary animate-spin" />
                        </div>
                    ) : error ? (
                        <div className="bg-red-50 border border-red-200 rounded-lg p-4 text-red-600">
                            <p className="font-medium">Erro ao carregar logs</p>
                            <p className="text-sm mt-1">{error}</p>
                        </div>
                    ) : filteredLogs.length === 0 ? (
                        <div className="text-center py-12 text-muted-foreground">
                            {searchTerm || levelFilter !== 'all'
                                ? 'Nenhum log encontrado com os filtros aplicados'
                                : 'Nenhum log disponível'}
                        </div>
                    ) : (
                        filteredLogs.map((log, index) => (
                            <LogEntryComponent key={index} log={log} />
                        ))
                    )}
                </div>
            </div>
        </div>
    );
};

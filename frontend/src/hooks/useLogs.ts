import { useState, useEffect, useCallback } from 'react';
import { systemAPI } from '@/services/api';

export interface LogEntry {
    timestamp: Date | string;
    level: 'INFO' | 'WARNING' | 'ERROR' | 'DEBUG';
    message: string;
}

export const useLogs = (autoRefresh: boolean = false, refreshInterval: number = 5000) => {
    const [logs, setLogs] = useState<LogEntry[]>([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState<string | null>(null);

    const fetchLogs = useCallback(async (limit?: number) => {
        try {
            setLoading(true);
            setError(null);
            const data = await systemAPI.getLogs(limit);
            setLogs(data as LogEntry[]);
        } catch (err) {
            const errorMessage = err instanceof Error ? err.message : 'Erro ao carregar logs';
            setError(errorMessage);
            console.error('Error fetching logs:', err);
        } finally {
            setLoading(false);
        }
    }, []);

    useEffect(() => {
        fetchLogs();

        if (autoRefresh) {
            const interval = setInterval(() => fetchLogs(), refreshInterval);
            return () => clearInterval(interval);
        }
    }, [fetchLogs, autoRefresh, refreshInterval]);

    return {
        logs,
        loading,
        error,
        fetchLogs,
    };
};

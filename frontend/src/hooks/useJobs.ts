import { useState, useEffect, useCallback } from 'react';
import type { PrintJob } from '@/types/index';
import { jobAPI } from '@/services/api';
import toast from 'react-hot-toast';

export const useJobs = () => {
    const [jobs, setJobs] = useState<PrintJob[]>([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState<string | null>(null);

    const fetchJobs = useCallback(async () => {
        try {
            setLoading(true);
            setError(null);
            const data = await jobAPI.getAll();
            setJobs(data);
        } catch (err) {
            const errorMessage = err instanceof Error ? err.message : 'Erro ao carregar jobs';
            setError(errorMessage);
            console.error('Error fetching jobs:', err);
        } finally {
            setLoading(false);
        }
    }, []);

    const cancelJob = useCallback(async (id: string) => {
        try {
            await jobAPI.cancel(id);
            toast.success('Job cancelado com sucesso');
            await fetchJobs();
        } catch (err) {
            toast.error('Erro ao cancelar job');
            console.error('Error canceling job:', err);
        }
    }, [fetchJobs]);

    const retryJob = useCallback(async (id: string) => {
        try {
            await jobAPI.retry(id);
            toast.success('Job reenviado para processamento');
            await fetchJobs();
        } catch (err) {
            toast.error('Erro ao tentar novamente');
            console.error('Error retrying job:', err);
        }
    }, [fetchJobs]);

    const cancelMultiple = useCallback(async (ids: string[]) => {
        try {
            await jobAPI.cancelMultiple(ids);
            toast.success(`${ids.length} job(s) cancelado(s)`);
            await fetchJobs();
        } catch (err) {
            toast.error('Erro ao cancelar jobs');
            console.error('Error canceling multiple jobs:', err);
        }
    }, [fetchJobs]);

    useEffect(() => {
        fetchJobs();

        // Auto-refresh every 5 seconds
        const interval = setInterval(fetchJobs, 5000);

        return () => clearInterval(interval);
    }, [fetchJobs]);

    return {
        jobs,
        loading,
        error,
        fetchJobs,
        cancelJob,
        retryJob,
        cancelMultiple,
    };
};

import { useEffect, useState } from 'react';
import { PrinterCard } from '../components/PrinterCard';
import { Dropzone } from '../components/Dropzone';
import { PerformanceChart } from '../components/PerformanceChart';
import { Zap, Server } from 'lucide-react';
import { printerAPI, systemAPI } from '../services/api';
import type { Printer, DashboardStats } from '../types';

export const Dashboard = () => {
    const [printers, setPrinters] = useState<Printer[]>([]);
    const [stats, setStats] = useState<DashboardStats | null>(null);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState<string | null>(null);

    useEffect(() => {
        const fetchData = async () => {
            try {
                setLoading(true);
                const [printersData, statsData] = await Promise.all([
                    printerAPI.getAll(),
                    systemAPI.getStats(),
                ]);
                setPrinters(printersData);
                setStats(statsData);
                setError(null);
            } catch (err) {
                console.error('Error fetching dashboard data:', err);
                setError('Não foi possível conectar ao backend. Verifique se o servidor está rodando.');
            } finally {
                setLoading(false);
            }
        };

        fetchData();
        // Refresh data every 5 seconds
        const interval = setInterval(fetchData, 5000);
        return () => clearInterval(interval);
    }, []);

    const mapPrinterStatus = (status: string): 'online' | 'offline' | 'warning' | 'error' => {
        if (status === 'online') return 'online';
        if (status === 'offline') return 'offline';
        if (status === 'error') return 'error';
        return 'warning';
    };

    if (loading && printers.length === 0) {
        return (
            <div className="flex items-center justify-center h-96">
                <div className="text-center">
                    <div className="w-16 h-16 border-4 border-primary border-t-transparent rounded-full animate-spin mx-auto mb-4"></div>
                    <p className="text-muted-foreground">Carregando dados...</p>
                </div>
            </div>
        );
    }

    if (error) {
        return (
            <div className="flex items-center justify-center h-96">
                <div className="text-center max-w-md">
                    <div className="w-16 h-16 bg-destructive/10 rounded-full flex items-center justify-center mx-auto mb-4">
                        <Server className="text-destructive" size={32} />
                    </div>
                    <h3 className="text-lg font-semibold text-foreground mb-2">Erro de Conexão</h3>
                    <p className="text-muted-foreground text-sm">{error}</p>
                </div>
            </div>
        );
    }

    return (
        <div className="space-y-6">
            {/* Header Section */}
            <div className="flex items-center justify-between">
                <div>
                    <h2 className="text-3xl font-bold text-foreground tracking-tight">Dashboard</h2>
                    <p className="text-muted-foreground mt-1">System Overview & Control</p>
                </div>
                <div className="flex gap-4">
                    <div className="flex items-center gap-2 px-4 py-2 rounded-lg bg-card border border-border">
                        <Server size={16} className="text-primary" />
                        <span className="text-sm font-medium">
                            {stats ? `${stats.printersOnline}/${stats.printersTotal} Online` : 'Loading...'}
                        </span>
                    </div>
                    <div className="flex items-center gap-2 px-4 py-2 rounded-lg bg-card border border-border">
                        <Zap size={16} className="text-success" />
                        <span className="text-sm font-medium">System Online</span>
                    </div>
                </div>
            </div>

            {/* Main Grid */}
            <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
                {/* Left Column: Printers & Dropzone */}
                <div className="lg:col-span-2 space-y-6">
                    {/* Dropzone Area */}
                    <Dropzone />

                    {/* Printers Grid */}
                    <div>
                        <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
                            <span className="w-1 h-5 bg-primary rounded-full" />
                            Active Printers
                        </h3>
                        {printers.length === 0 ? (
                            <div className="p-12 text-center border border-dashed border-border rounded-xl bg-card/50">
                                <p className="text-muted-foreground">Nenhuma impressora encontrada. Execute a descoberta de impressoras.</p>
                            </div>
                        ) : (
                            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                                {printers.map((printer) => (
                                    <PrinterCard
                                        key={printer.id}
                                        name={printer.name}
                                        status={mapPrinterStatus(printer.status)}
                                        inkLevel={Math.floor(Math.random() * 100)} // TODO: Get real ink level from backend
                                        jobsInQueue={printer.currentJobs}
                                    />
                                ))}
                            </div>
                        )}
                    </div>
                </div>

                {/* Right Column: Performance & Stats */}
                <div className="space-y-6">
                    {/* Performance Chart */}
                    <div className="p-6 rounded-xl bg-card/80 backdrop-blur-sm border border-border">
                        <div className="flex items-center justify-between mb-6">
                            <h3 className="font-semibold text-foreground">Processing Speed</h3>
                            <span className="text-xs text-success bg-success/10 px-2 py-1 rounded-full">
                                Real-time
                            </span>
                        </div>
                        <PerformanceChart />
                        <div className="grid grid-cols-2 gap-4 mt-6 pt-6 border-t border-border/50">
                            <div>
                                <p className="text-xs text-muted-foreground">Jobs Pending</p>
                                <p className="text-xl font-bold text-foreground">{stats?.jobsPending || 0}</p>
                            </div>
                            <div>
                                <p className="text-xs text-muted-foreground">Processing</p>
                                <p className="text-xl font-bold text-foreground">{stats?.jobsProcessing || 0}</p>
                            </div>
                        </div>
                    </div>

                    {/* Stats Summary */}
                    <div className="p-6 rounded-xl bg-card/80 backdrop-blur-sm border border-border">
                        <h3 className="font-semibold text-foreground mb-4">Statistics</h3>
                        <div className="space-y-4">
                            <div className="flex justify-between items-center">
                                <span className="text-sm text-muted-foreground">Completed Today</span>
                                <span className="text-lg font-bold text-success">{stats?.jobsCompleted || 0}</span>
                            </div>
                            <div className="flex justify-between items-center">
                                <span className="text-sm text-muted-foreground">Failed</span>
                                <span className="text-lg font-bold text-destructive">{stats?.jobsFailed || 0}</span>
                            </div>
                            <div className="flex justify-between items-center">
                                <span className="text-sm text-muted-foreground">Pages Printed</span>
                                <span className="text-lg font-bold text-foreground">{stats?.pagesTotal || 0}</span>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};

import React, { useState, useMemo } from 'react';
import { useJobs } from '@/hooks/useJobs';
import { JobCard } from '@/components/JobCard';
import type { PrintJob } from '@/types';
import { Search, Filter, RefreshCw, Loader2, X, Calendar, Printer, FileText, Clock } from 'lucide-react';
import { Dialog, DialogContent, DialogHeader, DialogFooter } from '@/components/ui/Dialog';
import { format } from 'date-fns';

export const JobsPage: React.FC = () => {
    const { jobs, loading, error, fetchJobs, cancelJob, retryJob } = useJobs();
    const [searchTerm, setSearchTerm] = useState('');
    const [statusFilter, setStatusFilter] = useState<PrintJob['status'] | 'all'>('all');
    const [selectedJob, setSelectedJob] = useState<PrintJob | null>(null);

    const filteredJobs = useMemo(() => {
        return jobs.filter(job => {
            const matchesSearch = job.fileName.toLowerCase().includes(searchTerm.toLowerCase()) ||
                job.printerName?.toLowerCase().includes(searchTerm.toLowerCase());
            const matchesStatus = statusFilter === 'all' || job.status === statusFilter;
            return matchesSearch && matchesStatus;
        });
    }, [jobs, searchTerm, statusFilter]);

    const stats = useMemo(() => {
        return {
            total: jobs.length,
            pending: jobs.filter(j => j.status === 'pending').length,
            processing: jobs.filter(j => j.status === 'processing').length,
            completed: jobs.filter(j => j.status === 'completed').length,
            failed: jobs.filter(j => j.status === 'failed').length,
            cancelled: jobs.filter(j => j.status === 'cancelled').length,
        };
    }, [jobs]);

    const handleViewDetails = (id: string) => {
        const job = jobs.find(j => j.id === id);
        if (job) {
            setSelectedJob(job);
        }
    };

    const formatDate = (date: Date | string | undefined) => {
        if (!date) return 'N/A';
        try {
            const dateObj = typeof date === 'string' ? new Date(date) : date;
            return format(dateObj, "dd/MM/yyyy 'às' HH:mm:ss");
        } catch {
            return 'N/A';
        }
    };

    const getStatusColor = (status: PrintJob['status']) => {
        switch (status) {
            case 'completed': return 'text-green-600 bg-green-50';
            case 'failed': return 'text-red-600 bg-red-50';
            case 'cancelled': return 'text-gray-600 bg-gray-50';
            case 'processing': return 'text-blue-600 bg-blue-50';
            case 'pending': return 'text-yellow-600 bg-yellow-50';
            default: return 'text-gray-600 bg-gray-50';
        }
    };

    return (
        <div className="space-y-6">
            {/* Header */}
            <div className="flex items-center justify-between">
                <div>
                    <h2 className="text-3xl font-bold text-foreground tracking-tight">Fila de Jobs</h2>
                    <p className="text-muted-foreground mt-1">Gerenciamento de impressões</p>
                </div>
                <button
                    onClick={fetchJobs}
                    disabled={loading}
                    className="flex items-center gap-2 px-4 py-2 bg-primary text-primary-foreground rounded-lg hover:bg-primary/90 transition-colors disabled:opacity-50"
                >
                    <RefreshCw className={`w-4 h-4 ${loading ? 'animate-spin' : ''}`} />
                    Atualizar
                </button>
            </div>

            {/* Stats Cards */}
            <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4">
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Total</div>
                    <div className="text-2xl font-bold text-foreground">{stats.total}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Pendentes</div>
                    <div className="text-2xl font-bold text-yellow-600">{stats.pending}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Processando</div>
                    <div className="text-2xl font-bold text-blue-600">{stats.processing}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Concluídos</div>
                    <div className="text-2xl font-bold text-green-600">{stats.completed}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Falharam</div>
                    <div className="text-2xl font-bold text-red-600">{stats.failed}</div>
                </div>
                <div className="bg-card border border-border rounded-lg p-4">
                    <div className="text-sm text-muted-foreground mb-1">Cancelados</div>
                    <div className="text-2xl font-bold text-gray-600">{stats.cancelled}</div>
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
                            placeholder="Buscar por nome do arquivo ou impressora..."
                            value={searchTerm}
                            onChange={(e) => setSearchTerm(e.target.value)}
                            className="w-full pl-10 pr-4 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                        />
                    </div>

                    {/* Status Filter */}
                    <div className="flex items-center gap-2">
                        <Filter className="w-4 h-4 text-muted-foreground" />
                        <select
                            value={statusFilter}
                            onChange={(e) => setStatusFilter(e.target.value as PrintJob['status'] | 'all')}
                            className="px-4 py-2 bg-background border border-border rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                        >
                            <option value="all">Todos os Status</option>
                            <option value="pending">Pendentes</option>
                            <option value="processing">Processando</option>
                            <option value="completed">Concluídos</option>
                            <option value="failed">Falharam</option>
                            <option value="cancelled">Cancelados</option>
                        </select>
                    </div>
                </div>
            </div>

            {/* Jobs List */}
            <div className="space-y-3">
                {loading && jobs.length === 0 ? (
                    <div className="flex items-center justify-center py-12">
                        <Loader2 className="w-8 h-8 text-primary animate-spin" />
                    </div>
                ) : error ? (
                    <div className="bg-red-50 border border-red-200 rounded-lg p-4 text-red-600">
                        <p className="font-medium">Erro ao carregar jobs</p>
                        <p className="text-sm mt-1">{error}</p>
                    </div>
                ) : filteredJobs.length === 0 ? (
                    <div className="bg-card border border-dashed border-border rounded-lg p-12 text-center">
                        <p className="text-muted-foreground">
                            {searchTerm || statusFilter !== 'all'
                                ? 'Nenhum job encontrado com os filtros aplicados'
                                : 'Nenhum job de impressão encontrado'}
                        </p>
                    </div>
                ) : (
                    filteredJobs.map(job => (
                        <JobCard
                            key={job.id}
                            job={job}
                            onCancel={cancelJob}
                            onRetry={retryJob}
                            onViewDetails={handleViewDetails}
                        />
                    ))
                )}
            </div>

            {/* Job Details Modal */}
            <Dialog open={!!selectedJob} onOpenChange={(open) => !open && setSelectedJob(null)}>
                {selectedJob && (
                    <DialogContent className="max-w-2xl">
                        <DialogHeader>
                            <div className="flex items-center justify-between">
                                <h3 className="text-xl font-bold">Detalhes do Job</h3>
                                <button
                                    onClick={() => setSelectedJob(null)}
                                    className="text-muted-foreground hover:text-foreground"
                                >
                                    <X className="w-5 h-5" />
                                </button>
                            </div>
                        </DialogHeader>

                        <div className="space-y-4">
                            {/* Status */}
                            <div className="flex items-center gap-2">
                                <span className="text-sm font-medium">Status:</span>
                                <span className={`px-3 py-1 rounded-full text-sm font-medium ${getStatusColor(selectedJob.status)}`}>
                                    {selectedJob.status === 'completed' && 'Concluído'}
                                    {selectedJob.status === 'failed' && 'Falhou'}
                                    {selectedJob.status === 'cancelled' && 'Cancelado'}
                                    {selectedJob.status === 'processing' && 'Processando'}
                                    {selectedJob.status === 'pending' && 'Pendente'}
                                </span>
                            </div>

                            {/* File Info */}
                            <div className="grid grid-cols-2 gap-4">
                                <div className="flex items-start gap-3">
                                    <FileText className="w-5 h-5 text-muted-foreground mt-0.5" />
                                    <div>
                                        <p className="text-sm text-muted-foreground">Arquivo</p>
                                        <p className="font-medium">{selectedJob.fileName}</p>
                                    </div>
                                </div>

                                <div className="flex items-start gap-3">
                                    <Printer className="w-5 h-5 text-muted-foreground mt-0.5" />
                                    <div>
                                        <p className="text-sm text-muted-foreground">Impressora</p>
                                        <p className="font-medium">{selectedJob.printerName || 'N/A'}</p>
                                    </div>
                                </div>
                            </div>

                            {/* Dates */}
                            <div className="grid grid-cols-2 gap-4">
                                <div className="flex items-start gap-3">
                                    <Calendar className="w-5 h-5 text-muted-foreground mt-0.5" />
                                    <div>
                                        <p className="text-sm text-muted-foreground">Criado em</p>
                                        <p className="font-medium">{formatDate(selectedJob.createdAt)}</p>
                                    </div>
                                </div>

                                {selectedJob.startedAt && (
                                    <div className="flex items-start gap-3">
                                        <Clock className="w-5 h-5 text-muted-foreground mt-0.5" />
                                        <div>
                                            <p className="text-sm text-muted-foreground">Iniciado em</p>
                                            <p className="font-medium">{formatDate(selectedJob.startedAt)}</p>
                                        </div>
                                    </div>
                                )}

                                {selectedJob.completedAt && (
                                    <div className="flex items-start gap-3">
                                        <Clock className="w-5 h-5 text-muted-foreground mt-0.5" />
                                        <div>
                                            <p className="text-sm text-muted-foreground">Concluído em</p>
                                            <p className="font-medium">{formatDate(selectedJob.completedAt)}</p>
                                        </div>
                                    </div>
                                )}
                            </div>

                            {/* Progress */}
                            {selectedJob.progress !== undefined && (
                                <div>
                                    <div className="flex items-center justify-between mb-2">
                                        <span className="text-sm font-medium">Progresso</span>
                                        <span className="text-sm text-muted-foreground">{selectedJob.progress}%</span>
                                    </div>
                                    <div className="w-full bg-gray-200 rounded-full h-2">
                                        <div
                                            className="bg-primary h-2 rounded-full transition-all"
                                            style={{ width: `${selectedJob.progress}%` }}
                                        />
                                    </div>
                                </div>
                            )}

                            {/* Options */}
                            {selectedJob.options && (
                                <div className="border-t border-border pt-4">
                                    <p className="text-sm font-medium mb-2">Opções de Impressão</p>
                                    <div className="grid grid-cols-2 gap-2 text-sm">
                                        {selectedJob.options.copies && (
                                            <div>
                                                <span className="text-muted-foreground">Cópias: </span>
                                                <span className="font-medium">{selectedJob.options.copies}</span>
                                            </div>
                                        )}
                                        {selectedJob.options.colorMode && (
                                            <div>
                                                <span className="text-muted-foreground">Modo de cor: </span>
                                                <span className="font-medium">{selectedJob.options.colorMode}</span>
                                            </div>
                                        )}
                                        {selectedJob.options.duplex && (
                                            <div>
                                                <span className="text-muted-foreground">Frente e verso: </span>
                                                <span className="font-medium">{selectedJob.options.duplex ? 'Sim' : 'Não'}</span>
                                            </div>
                                        )}
                                        {selectedJob.options.paperSize && (
                                            <div>
                                                <span className="text-muted-foreground">Tamanho do papel: </span>
                                                <span className="font-medium">{selectedJob.options.paperSize}</span>
                                            </div>
                                        )}
                                    </div>
                                </div>
                            )}

                            {/* Error Message */}
                            {selectedJob.error && (
                                <div className="bg-red-50 border border-red-200 rounded-lg p-3">
                                    <p className="text-sm font-medium text-red-800 mb-1">Erro</p>
                                    <p className="text-sm text-red-600">{selectedJob.error}</p>
                                </div>
                            )}
                        </div>

                        <DialogFooter>
                            <button
                                onClick={() => setSelectedJob(null)}
                                className="px-4 py-2 bg-card border border-border rounded-lg hover:bg-muted transition-colors"
                            >
                                Fechar
                            </button>
                            {selectedJob.status === 'failed' && (
                                <button
                                    onClick={() => {
                                        retryJob(selectedJob.id);
                                        setSelectedJob(null);
                                    }}
                                    className="px-4 py-2 bg-primary text-primary-foreground rounded-lg hover:bg-primary/90 transition-colors"
                                >
                                    Tentar Novamente
                                </button>
                            )}
                        </DialogFooter>
                    </DialogContent>
                )}
            </Dialog>
        </div>
    );
};

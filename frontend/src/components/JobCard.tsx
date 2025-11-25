import React from 'react';
import type { PrintJob } from '@/types';
import { FileText, Clock, CheckCircle, XCircle, AlertCircle, Loader2, Printer } from 'lucide-react';
import { formatDistanceToNow } from 'date-fns';
import { ptBR } from 'date-fns/locale';

interface JobCardProps {
    job: PrintJob;
    onCancel?: (id: string) => void;
    onRetry?: (id: string) => void;
    onViewDetails?: (id: string) => void;
}

export const JobCard: React.FC<JobCardProps> = ({ job, onCancel, onRetry, onViewDetails }) => {
    const getStatusIcon = () => {
        switch (job.status) {
            case 'pending':
                return <Clock className="w-5 h-5 text-yellow-500" />;
            case 'processing':
                return <Loader2 className="w-5 h-5 text-blue-500 animate-spin" />;
            case 'completed':
                return <CheckCircle className="w-5 h-5 text-green-500" />;
            case 'failed':
                return <XCircle className="w-5 h-5 text-red-500" />;
            case 'cancelled':
                return <AlertCircle className="w-5 h-5 text-gray-500" />;
            default:
                return <FileText className="w-5 h-5 text-gray-400" />;
        }
    };

    const getStatusColor = () => {
        switch (job.status) {
            case 'pending':
                return 'bg-yellow-500/10 text-yellow-700 border-yellow-500/20';
            case 'processing':
                return 'bg-blue-500/10 text-blue-700 border-blue-500/20';
            case 'completed':
                return 'bg-green-500/10 text-green-700 border-green-500/20';
            case 'failed':
                return 'bg-red-500/10 text-red-700 border-red-500/20';
            case 'cancelled':
                return 'bg-gray-500/10 text-gray-700 border-gray-500/20';
            default:
                return 'bg-gray-500/10 text-gray-700 border-gray-500/20';
        }
    };

    const getStatusLabel = () => {
        switch (job.status) {
            case 'pending':
                return 'Pendente';
            case 'processing':
                return 'Processando';
            case 'completed':
                return 'Concluído';
            case 'failed':
                return 'Falhou';
            case 'cancelled':
                return 'Cancelado';
            default:
                return job.status;
        }
    };

    const formatFileSize = (bytes: number) => {
        if (bytes < 1024) return `${bytes} B`;
        if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
        return `${(bytes / (1024 * 1024)).toFixed(1)} MB`;
    };

    const createdDate = typeof job.createdAt === 'string' ? new Date(job.createdAt) : job.createdAt;

    return (
        <div className="bg-card border border-border rounded-lg p-4 hover:shadow-md transition-shadow">
            <div className="flex items-start justify-between gap-4">
                <div className="flex items-start gap-3 flex-1 min-w-0">
                    {getStatusIcon()}
                    <div className="flex-1 min-w-0">
                        <div className="flex items-center gap-2 mb-1">
                            <h3 className="font-medium text-foreground truncate">{job.fileName}</h3>
                            <span className={`px-2 py-0.5 text-xs font-medium rounded-full border ${getStatusColor()}`}>
                                {getStatusLabel()}
                            </span>
                        </div>

                        <div className="flex items-center gap-4 text-sm text-muted-foreground mb-2">
                            <div className="flex items-center gap-1">
                                <Printer className="w-4 h-4" />
                                <span>{job.printerName || 'Impressora não especificada'}</span>
                            </div>
                            <span>•</span>
                            <span>{formatFileSize(job.fileSize)}</span>
                            {job.totalPages && (
                                <>
                                    <span>•</span>
                                    <span>{job.totalPages} página{job.totalPages > 1 ? 's' : ''}</span>
                                </>
                            )}
                        </div>

                        {job.status === 'processing' && job.progress !== undefined && (
                            <div className="mb-2">
                                <div className="flex items-center justify-between text-xs text-muted-foreground mb-1">
                                    <span>Progresso</span>
                                    <span>{Math.round(job.progress * 100)}%</span>
                                </div>
                                <div className="w-full bg-muted rounded-full h-2 overflow-hidden">
                                    <div
                                        className="bg-blue-500 h-full transition-all duration-300 rounded-full"
                                        style={{ width: `${job.progress * 100}%` }}
                                    />
                                </div>
                            </div>
                        )}

                        {job.errorMessage && (
                            <div className="text-sm text-red-600 bg-red-50 border border-red-200 rounded px-2 py-1 mt-2">
                                {job.errorMessage}
                            </div>
                        )}

                        <div className="text-xs text-muted-foreground mt-2">
                            Criado {formatDistanceToNow(createdDate, { addSuffix: true, locale: ptBR })}
                        </div>
                    </div>
                </div>

                <div className="flex gap-2">
                    {job.status === 'pending' && onCancel && (
                        <button
                            onClick={() => onCancel(job.id)}
                            className="px-3 py-1.5 text-sm font-medium text-red-600 hover:bg-red-50 border border-red-200 rounded-md transition-colors"
                        >
                            Cancelar
                        </button>
                    )}
                    {job.status === 'failed' && onRetry && (
                        <button
                            onClick={() => onRetry(job.id)}
                            className="px-3 py-1.5 text-sm font-medium text-blue-600 hover:bg-blue-50 border border-blue-200 rounded-md transition-colors"
                        >
                            Tentar Novamente
                        </button>
                    )}
                    {onViewDetails && (
                        <button
                            onClick={() => onViewDetails(job.id)}
                            className="px-3 py-1.5 text-sm font-medium text-foreground hover:bg-muted border border-border rounded-md transition-colors"
                        >
                            Detalhes
                        </button>
                    )}
                </div>
            </div>
        </div>
    );
};

import { memo } from 'react';
import { Card, CardContent } from '@/components/ui/Card';
import { Badge } from '@/components/ui/Badge';
import { Progress } from '@/components/ui/Progress';
import { Button } from '@/components/ui/Button';
import type { PrintJob } from '@/types';
import { formatBytes, formatRelativeTime, getStatusColor, cn } from '@/lib/utils';
import { FileText, Clock, RefreshCw, X } from 'lucide-react';

interface JobItemProps {
  job: PrintJob;
  onClick?: () => void;
  onRetry?: (jobId: string) => void;
  onCancel?: (jobId: string) => void;
}

export const JobItem = memo(({ job, onClick, onRetry, onCancel }: JobItemProps) => {
  const handleAction = (e: React.MouseEvent, action: () => void) => {
    e.stopPropagation();
    action();
  };

  return (
    <Card 
      className={cn(
        "transition-all duration-200 hover:shadow-md hover:scale-[1.01]",
        onClick && "cursor-pointer"
      )} 
      onClick={onClick}
      role={onClick ? 'button' : 'article'}
      tabIndex={onClick ? 0 : undefined}
      onKeyDown={onClick ? (e) => {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          onClick();
        }
      } : undefined}
      aria-label={onClick ? `Job ${job.fileName}, status: ${job.status}` : undefined}
    >
      <CardContent className="p-4">
        <div className="flex items-start justify-between gap-4">
          <div className="flex items-start gap-3 flex-1">
            <div className="rounded-md bg-primary/10 p-2 transition-colors hover:bg-primary/20">
              <FileText className="h-5 w-5 text-primary" aria-hidden="true" />
            </div>
            
            <div className="flex-1 space-y-1">
              <div className="flex items-center gap-2 flex-wrap">
                <h4 className="font-medium text-sm">{job.fileName}</h4>
                <Badge 
                  className={getStatusColor(job.status)}
                  aria-label={`Status: ${job.status}`}
                >
                  {job.status}
                </Badge>
              </div>
              
              <div className="flex items-center gap-4 text-xs text-muted-foreground flex-wrap">
                <span>Impressora: {job.printerName}</span>
                <span aria-hidden="true">•</span>
                <span>{formatBytes(job.fileSize)}</span>
                {job.totalPages && (
                  <>
                    <span aria-hidden="true">•</span>
                    <span>{job.totalPages} páginas</span>
                  </>
                )}
              </div>

              {job.status === 'processing' && (
                <div className="mt-2 space-y-1" role="progressbar" aria-valuenow={job.progress} aria-valuemin={0} aria-valuemax={100}>
                  <div className="flex items-center justify-between text-xs">
                    <span className="text-muted-foreground">
                      Progresso: {job.currentPage || 0}/{job.totalPages || 0}
                    </span>
                    <span className="font-medium">{job.progress}%</span>
                  </div>
                  <Progress value={job.progress} aria-label={`Progresso: ${job.progress}%`} />
                </div>
              )}

              <div className="flex items-center gap-2 text-xs text-muted-foreground">
                <Clock className="h-3 w-3" aria-hidden="true" />
                <time dateTime={typeof job.createdAt === 'string' ? job.createdAt : job.createdAt.toISOString()}>
                  {formatRelativeTime(typeof job.createdAt === 'string' ? new Date(job.createdAt) : job.createdAt)}
                </time>
              </div>
            </div>
          </div>
          
          {/* Action Buttons */}
          <div className="flex items-center gap-2">
            {(job.status === 'failed' || job.status === 'cancelled') && onRetry && (
              <Button
                size="sm"
                variant="outline"
                onClick={(e) => handleAction(e, () => onRetry(job.id))}
                title="Tentar novamente"
                aria-label="Tentar imprimir novamente"
              >
                <RefreshCw className="h-4 w-4" />
              </Button>
            )}
            {(job.status === 'pending' || job.status === 'processing') && onCancel && (
              <Button
                size="sm"
                variant="outline"
                onClick={(e) => handleAction(e, () => onCancel(job.id))}
                title="Cancelar"
                aria-label="Cancelar impressão"
              >
                <X className="h-4 w-4" />
              </Button>
            )}
          </div>
        </div>
      </CardContent>
    </Card>
  );
});

JobItem.displayName = 'JobItem';

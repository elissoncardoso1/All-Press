import { memo } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/Card';
import { Badge } from '@/components/ui/Badge';
import type { Printer as PrinterType } from '@/types';
import { getStatusBadgeColor, formatRelativeTime, cn } from '@/lib/utils';

interface PrinterCardProps {
  printer: PrinterType;
  onClick?: () => void;
}

export const PrinterCard = memo(({ printer, onClick }: PrinterCardProps) => {
  return (
    <Card 
      className={cn(
        "transition-all duration-200 hover:shadow-md hover:scale-[1.01]",
        "overflow-hidden", // Prevent content overflow
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
      aria-label={onClick ? `Impressora ${printer.name}, status: ${printer.status}` : undefined}
    >
      <CardHeader className="pb-3">
        <div className="flex items-start justify-between gap-2">
          <div className="flex-1 min-w-0">
            <CardTitle className="text-base truncate" title={printer.name}>
              {printer.name}
            </CardTitle>
            <p 
              className="text-xs text-muted-foreground mt-1 break-words overflow-hidden"
              style={{
                display: '-webkit-box',
                WebkitLineClamp: 2,
                WebkitBoxOrient: 'vertical',
                lineHeight: '1.4',
                maxHeight: '2.8em'
              }}
              title={printer.manufacturer === printer.model 
                ? printer.manufacturer 
                : `${printer.manufacturer} ${printer.model}`}
            >
              {printer.manufacturer === printer.model 
                ? printer.manufacturer 
                : `${printer.manufacturer} ${printer.model}`}
            </p>
          </div>
          <div className="flex items-center gap-2 flex-shrink-0">
            <div 
              className={cn("h-2 w-2 rounded-full flex-shrink-0", getStatusBadgeColor(printer.status))}
              aria-hidden="true"
            />
            <Badge variant="outline" className="text-xs whitespace-nowrap" aria-label={`Status: ${printer.status}`}>
              {printer.status}
            </Badge>
          </div>
        </div>
      </CardHeader>
      <CardContent>
        <div className="space-y-2 text-sm">
          {printer.location && (
            <div className="flex justify-between gap-2">
              <span className="text-muted-foreground whitespace-nowrap">Localização:</span>
              <span className="font-medium text-right truncate" title={printer.location}>
                {printer.location}
              </span>
            </div>
          )}
          <div className="flex justify-between gap-2">
            <span className="text-muted-foreground whitespace-nowrap">Jobs em fila:</span>
            <span className="font-medium">{printer.currentJobs}</span>
          </div>
          <div className="flex justify-between gap-2">
            <span className="text-muted-foreground whitespace-nowrap">Total processado:</span>
            <span className="font-medium">{printer.totalJobsProcessed}</span>
          </div>
          {printer.lastActivity && (
            <div className="flex justify-between gap-2 text-xs">
              <span className="text-muted-foreground whitespace-nowrap">Última atividade:</span>
              <span className="text-right">{formatRelativeTime(new Date(printer.lastActivity))}</span>
            </div>
          )}
        </div>
      </CardContent>
    </Card>
  );
});

PrinterCard.displayName = 'PrinterCard';

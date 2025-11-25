import { useSystemStore } from '@/store/systemStore';
import { usePrinterStore } from '@/store/printerStore';
import { useJobStore } from '@/store/jobStore';
import { Badge } from '../ui/Badge';
import { Activity } from 'lucide-react';

export const Footer = () => {
  const status = useSystemStore((state) => state.status);
  const stats = useSystemStore((state) => state.stats);
  const printers = usePrinterStore((state) => state.printers);
  const jobs = useJobStore((state) => state.jobs);

  const onlinePrinters = printers.filter((p) => p.status === 'online').length;
  const activeJobs = jobs.filter((j) => j.status === 'processing').length;

  return (
    <footer 
      className="sticky bottom-0 z-40 w-full border-t bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60"
      role="contentinfo"
      aria-label="Rodapé do sistema"
    >
      <div className="container flex h-12 flex-col items-center justify-between gap-2 px-4 text-sm md:flex-row">
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2">
            <Activity className="h-4 w-4" aria-hidden="true" />
            <span className="text-muted-foreground">Status:</span>
            <Badge 
              variant={status?.status === 'online' ? 'success' : 'destructive'}
              aria-label={`Status do sistema: ${status?.status || 'offline'}`}
            >
              {status?.status || 'offline'}
            </Badge>
          </div>
        </div>

        <div className="flex items-center gap-4 text-muted-foreground flex-wrap justify-center md:gap-6">
          <span>
            Impressoras: <strong className="text-foreground" aria-label={`${onlinePrinters} de ${printers.length} impressoras online`}>{onlinePrinters}/{printers.length}</strong>
          </span>
          <span>
            Jobs Ativos: <strong className="text-foreground" aria-label={`${activeJobs} jobs ativos`}>{activeJobs}</strong>
          </span>
          <span>
            Páginas Hoje: <strong className="text-foreground" aria-label={`${stats?.pagesToday || 0} páginas impressas hoje`}>{stats?.pagesToday || 0}</strong>
          </span>
        </div>
      </div>
    </footer>
  );
};

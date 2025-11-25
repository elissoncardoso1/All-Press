import { useEffect, useMemo } from 'react';
import { Printer, FileText, CheckCircle, TrendingUp, DollarSign, AlertCircle } from 'lucide-react';
import { StatsCard } from './StatsCard';
import { PrinterCard } from './PrinterCard';
import { JobItem } from './JobItem';
import { useSystemStore } from '@/store/systemStore';
import { usePrinterStore } from '@/store/printerStore';
import { useJobStore } from '@/store/jobStore';
import { formatCurrency } from '@/lib/utils';
import { Skeleton } from '@/components/ui/Skeleton';
import { Alert } from '@/components/ui/Alert';
import { Card, CardContent } from '@/components/ui/Card';

export const Dashboard = () => {
  const stats = useSystemStore((state) => state.stats);
  const isLoadingStats = useSystemStore((state) => state.loading);
  const errorStats = useSystemStore((state) => state.error);
  const fetchStats = useSystemStore((state) => state.fetchStats);
  const fetchStatus = useSystemStore((state) => state.fetchStatus);
  
  const printers = usePrinterStore((state) => state.printers);
  const isLoadingPrinters = usePrinterStore((state) => state.loading);
  const errorPrinters = usePrinterStore((state) => state.error);
  const fetchPrinters = usePrinterStore((state) => state.fetchPrinters);
  
  const jobs = useJobStore((state) => state.jobs);
  const isLoadingJobs = useJobStore((state) => state.loading);
  const errorJobs = useJobStore((state) => state.error);
  const fetchJobs = useJobStore((state) => state.fetchJobs);
  const retryJob = useJobStore((state) => state.retryJob);
  const cancelJob = useJobStore((state) => state.cancelJob);

  // Fetch data on mount only
  useEffect(() => {
    fetchStats();
    fetchStatus();
    fetchPrinters();
    fetchJobs();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Memoize computed values to prevent unnecessary recalculations
  const onlinePrinters = useMemo(
    () => printers.filter((p) => p.status === 'online'),
    [printers]
  );

  const recentJobs = useMemo(
    () => jobs.slice(0, 5),
    [jobs]
  );

  const activeJobs = useMemo(
    () => jobs.filter((j) => j.status === 'processing'),
    [jobs]
  );

  const isLoading = isLoadingStats || isLoadingPrinters || isLoadingJobs;
  const hasError = errorStats || errorPrinters || errorJobs;

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold tracking-tight">Dashboard</h1>
        <p className="text-muted-foreground">Visão geral do sistema de impressão</p>
      </div>

      {/* Error States */}
      {hasError && (
        <Alert variant="error">
          <AlertCircle className="h-4 w-4" />
          <div>
            <p className="font-semibold">Erro ao carregar dados</p>
            <p className="text-sm">
              {errorStats && 'Não foi possível carregar estatísticas. '}
              {errorPrinters && 'Não foi possível carregar impressoras. '}
              {errorJobs && 'Não foi possível carregar jobs.'}
            </p>
          </div>
        </Alert>
      )}

      {/* Statistics Cards */}
      <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
        {isLoading ? (
          Array.from({ length: 4 }).map((_, i) => (
            <Card key={i} className="p-6">
              <Skeleton className="h-4 w-24 mb-4" />
              <Skeleton className="h-8 w-16 mb-2" />
              <Skeleton className="h-3 w-32" />
            </Card>
          ))
        ) : (
          <>
            <StatsCard
              title="Impressoras Online"
              value={`${stats?.printersOnline || 0}/${stats?.printersTotal || 0}`}
              icon={Printer}
              description={`${stats?.printersOnline || 0} impressoras ativas`}
              isLoading={isLoadingStats}
            />
            <StatsCard
              title="Jobs em Processamento"
              value={stats?.jobsProcessing || 0}
              icon={FileText}
              description={`${stats?.jobsPending || 0} na fila`}
              isLoading={isLoadingJobs}
            />
            <StatsCard
              title="Jobs Concluídos"
              value={stats?.jobsCompleted || 0}
              icon={CheckCircle}
              description={`${stats?.jobsFailed || 0} falharam`}
              isLoading={isLoadingJobs}
            />
            <StatsCard
              title="Páginas Hoje"
              value={stats?.pagesToday || 0}
              icon={TrendingUp}
              description={`${stats?.pagesTotal || 0} no total`}
              isLoading={isLoadingStats}
            />
          </>
        )}
      </div>

      <div className="grid gap-6 lg:grid-cols-2">
        {/* Active Printers */}
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <h2 className="text-xl font-semibold">Impressoras Ativas</h2>
            {!isLoadingPrinters && (
              <span className="text-sm text-muted-foreground">
                {onlinePrinters.length} online
              </span>
            )}
          </div>
          {isLoadingPrinters ? (
            <div className="grid gap-4">
              {Array.from({ length: 3 }).map((_, i) => (
                <Card key={i} className="p-4">
                  <Skeleton className="h-5 w-32 mb-3" />
                  <Skeleton className="h-4 w-24 mb-2" />
                  <Skeleton className="h-4 w-full" />
                </Card>
              ))}
            </div>
          ) : onlinePrinters.length === 0 ? (
            <Card>
              <CardContent className="p-6 text-center text-muted-foreground">
                <Printer className="h-12 w-12 mx-auto mb-2 opacity-50" />
                <p>Nenhuma impressora online no momento</p>
              </CardContent>
            </Card>
          ) : (
            <div className="grid gap-4">
              {onlinePrinters.slice(0, 3).map((printer) => (
                <PrinterCard key={printer.id} printer={printer} />
              ))}
            </div>
          )}
        </div>

        {/* Recent Jobs */}
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <h2 className="text-xl font-semibold">Jobs Recentes</h2>
            {!isLoadingJobs && (
              <span className="text-sm text-muted-foreground">
                {activeJobs.length} ativos
              </span>
            )}
          </div>
          {isLoadingJobs ? (
            <div className="space-y-3">
              {Array.from({ length: 5 }).map((_, i) => (
                <Card key={i} className="p-4">
                  <Skeleton className="h-5 w-48 mb-2" />
                  <Skeleton className="h-4 w-32 mb-2" />
                  <Skeleton className="h-3 w-full" />
                </Card>
              ))}
            </div>
          ) : recentJobs.length === 0 ? (
            <Card>
              <CardContent className="p-6 text-center text-muted-foreground">
                <FileText className="h-12 w-12 mx-auto mb-2 opacity-50" />
                <p>Nenhum job recente</p>
              </CardContent>
            </Card>
          ) : (
            <div className="space-y-3">
              {recentJobs.map((job) => (
                <JobItem 
                  key={job.id} 
                  job={job}
                  onRetry={retryJob}
                  onCancel={cancelJob}
                />
              ))}
            </div>
          )}
        </div>
      </div>

      {/* Cost Estimate */}
      {stats?.estimatedCost !== undefined && (
        <div className="rounded-lg border bg-gradient-to-r from-primary/10 to-primary/5 p-6">
          <div className="flex items-center gap-4">
            <div className="rounded-full bg-primary/10 p-4">
              <DollarSign className="h-8 w-8 text-primary" />
            </div>
            <div>
              <p className="text-sm text-muted-foreground">Custo Estimado (Este Mês)</p>
              <p className="text-3xl font-bold">{formatCurrency(stats.estimatedCost)}</p>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

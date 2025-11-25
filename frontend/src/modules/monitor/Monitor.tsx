import { useEffect } from 'react';
import { Activity, Cpu, HardDrive, Network } from 'lucide-react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/Card';
import { Badge } from '@/components/ui/Badge';
import { Progress } from '@/components/ui/Progress';
import { useSystemStore } from '@/store/systemStore';
import { formatDuration } from '@/lib/utils';

export const Monitor = () => {
  const metrics = useSystemStore((state) => state.metrics);
  const status = useSystemStore((state) => state.status);
  const fetchMetrics = useSystemStore((state) => state.fetchMetrics);
  const fetchStatus = useSystemStore((state) => state.fetchStatus);

  useEffect(() => {
    fetchMetrics();
    fetchStatus();

    // Update metrics every 5 seconds
    const interval = setInterval(() => {
      fetchMetrics();
      fetchStatus();
    }, 5000);

    return () => clearInterval(interval);
  }, [fetchMetrics, fetchStatus]);

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold tracking-tight">Monitor do Sistema</h1>
        <p className="text-muted-foreground">
          Acompanhe a performance e saúde do sistema em tempo real
        </p>
      </div>

      {/* System Status */}
      <Card>
        <CardHeader>
          <CardTitle>Status do Sistema</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
            <div className="space-y-2">
              <p className="text-sm text-muted-foreground">Status</p>
              <Badge variant={status?.status === 'online' ? 'success' : 'destructive'}>
                {status?.status || 'Desconhecido'}
              </Badge>
            </div>
            <div className="space-y-2">
              <p className="text-sm text-muted-foreground">Uptime</p>
              <p className="text-lg font-semibold">
                {status?.uptime ? formatDuration(status.uptime) : '—'}
              </p>
            </div>
            <div className="space-y-2">
              <p className="text-sm text-muted-foreground">Versão</p>
              <p className="text-lg font-semibold">{status?.version || '—'}</p>
            </div>
            <div className="space-y-2">
              <p className="text-sm text-muted-foreground">CUPS</p>
              <Badge variant={status?.cupsConnected ? 'success' : 'destructive'}>
                {status?.cupsConnected ? 'Conectado' : 'Desconectado'}
              </Badge>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Performance Metrics */}
      <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">CPU</CardTitle>
            <Cpu className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{(metrics?.cpuUsage ?? 0).toFixed(1)}%</div>
            <Progress value={metrics?.cpuUsage ?? 0} className="mt-2" />
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Memória</CardTitle>
            <HardDrive className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{(metrics?.memoryUsage ?? 0).toFixed(1)}%</div>
            <Progress value={metrics?.memoryUsage ?? 0} className="mt-2" />
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Conexões Ativas</CardTitle>
            <Network className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{metrics?.activeConnections ?? 0}</div>
            <p className="text-xs text-muted-foreground mt-2">
              {(metrics?.requestsPerSecond ?? 0).toFixed(1)} req/s
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Cache Hit Ratio</CardTitle>
            <Activity className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{(metrics?.cacheHitRatio ?? 0).toFixed(1)}%</div>
            <Progress value={metrics?.cacheHitRatio ?? 0} className="mt-2" />
          </CardContent>
        </Card>
      </div>

      {/* Thread Pool Status */}
      {metrics && metrics.threadPoolActive !== undefined && metrics.threadPoolMax !== undefined && (
        <Card>
          <CardHeader>
            <CardTitle>Thread Pool</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-4">
              <div className="flex items-center justify-between">
                <span className="text-sm text-muted-foreground">Threads Ativos</span>
                <span className="text-lg font-semibold">
                  {metrics.threadPoolActive ?? 0} / {metrics.threadPoolMax ?? 0}
                </span>
              </div>
              <Progress
                value={metrics.threadPoolMax > 0 ? ((metrics.threadPoolActive ?? 0) / metrics.threadPoolMax) * 100 : 0}
              />
              <div className="flex items-center justify-between text-sm">
                <span className="text-muted-foreground">Tempo Médio de Resposta</span>
                <span className="font-medium">{(metrics.averageResponseTime ?? 0).toFixed(2)}ms</span>
              </div>
            </div>
          </CardContent>
        </Card>
      )}
    </div>
  );
};

import { useEffect, useState } from 'react';
import { Search, RefreshCw, Plus, Grid3x3, List } from 'lucide-react';
import { Button } from '@/components/ui/Button';
import { Input } from '@/components/ui/Input';
import { Badge } from '@/components/ui/Badge';
import { PrinterCard } from '../dashboard/PrinterCard';
import { usePrinterStore } from '@/store/printerStore';
import { useFilteredPrinters } from '@/hooks/useFilteredPrinters';

export const Printers = () => {
  const [viewMode, setViewMode] = useState<'grid' | 'list'>('grid');
  const [search, setSearch] = useState('');
  
  const fetchPrinters = usePrinterStore((state) => state.fetchPrinters);
  const discoverPrinters = usePrinterStore((state) => state.discoverPrinters);
  const setFilters = usePrinterStore((state) => state.setFilters);
  const discovering = usePrinterStore((state) => state.discovering);
  const loading = usePrinterStore((state) => state.loading);

  const filteredPrinters = useFilteredPrinters();

  useEffect(() => {
    fetchPrinters();
  }, [fetchPrinters]);

  useEffect(() => {
    setFilters({ search });
  }, [search, setFilters]);

  const handleDiscover = async () => {
    await discoverPrinters();
  };

  const statusCounts = {
    online: filteredPrinters.filter((p) => p.status === 'online').length,
    offline: filteredPrinters.filter((p) => p.status === 'offline').length,
    error: filteredPrinters.filter((p) => p.status === 'error').length,
  };

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Impressoras</h1>
          <p className="text-muted-foreground">Gerencie todas as impressoras do sistema</p>
        </div>
        <div className="flex items-center gap-2">
          <Button onClick={handleDiscover} disabled={discovering}>
            <RefreshCw className={`mr-2 h-4 w-4 ${discovering ? 'animate-spin' : ''}`} />
            {discovering ? 'Descobrindo...' : 'Descobrir'}
          </Button>
          <Button>
            <Plus className="mr-2 h-4 w-4" />
            Adicionar
          </Button>
        </div>
      </div>

      {/* Status Overview */}
      <div className="flex items-center gap-4">
        <Badge variant="success" className="px-3 py-1">
          Online: {statusCounts.online}
        </Badge>
        <Badge variant="secondary" className="px-3 py-1">
          Offline: {statusCounts.offline}
        </Badge>
        <Badge variant="destructive" className="px-3 py-1">
          Erro: {statusCounts.error}
        </Badge>
      </div>

      {/* Filters and View Toggle */}
      <div className="flex items-center gap-4">
        <div className="relative flex-1 max-w-md">
          <Search className="absolute left-3 top-1/2 h-4 w-4 -translate-y-1/2 text-muted-foreground" />
          <Input
            placeholder="Buscar impressoras..."
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            className="pl-10"
          />
        </div>
        <div className="flex items-center gap-1 rounded-lg border p-1">
          <Button
            variant={viewMode === 'grid' ? 'secondary' : 'ghost'}
            size="sm"
            onClick={() => setViewMode('grid')}
          >
            <Grid3x3 className="h-4 w-4" />
          </Button>
          <Button
            variant={viewMode === 'list' ? 'secondary' : 'ghost'}
            size="sm"
            onClick={() => setViewMode('list')}
          >
            <List className="h-4 w-4" />
          </Button>
        </div>
      </div>

      {/* Printers Grid/List */}
      {loading ? (
        <div className="text-center py-12">
          <RefreshCw className="h-8 w-8 animate-spin mx-auto text-muted-foreground" />
          <p className="mt-4 text-muted-foreground">Carregando impressoras...</p>
        </div>
      ) : filteredPrinters.length === 0 ? (
        <div className="text-center py-12">
          <p className="text-muted-foreground">Nenhuma impressora encontrada</p>
        </div>
      ) : (
        <div
          className={
            viewMode === 'grid'
              ? 'grid gap-4 md:grid-cols-2 lg:grid-cols-3'
              : 'space-y-4'
          }
        >
          {filteredPrinters.map((printer) => (
            <PrinterCard key={printer.id} printer={printer} />
          ))}
        </div>
      )}
    </div>
  );
};

import { useEffect, useState } from 'react';
import { Search, X, Filter } from 'lucide-react';
import { Button } from '@/components/ui/Button';
import { Input } from '@/components/ui/Input';
import { Badge } from '@/components/ui/Badge';
import { JobItem } from '../dashboard/JobItem';
import { useJobStore } from '@/store/jobStore';
import { useFilteredJobs } from '@/hooks/useFilteredJobs';

export const Jobs = () => {
  const [search, setSearch] = useState('');
  
  const fetchJobs = useJobStore((state) => state.fetchJobs);
  const setFilters = useJobStore((state) => state.setFilters);
  const cancelMultipleJobs = useJobStore((state) => state.cancelMultipleJobs);
  const retryJob = useJobStore((state) => state.retryJob);
  const cancelJob = useJobStore((state) => state.cancelJob);
  const selectedJobs = useJobStore((state) => state.selectedJobs);
  const clearSelection = useJobStore((state) => state.clearSelection);
  const loading = useJobStore((state) => state.loading);

  const filteredJobs = useFilteredJobs();

  useEffect(() => {
    fetchJobs();
  }, [fetchJobs]);

  useEffect(() => {
    setFilters({ search });
  }, [search, setFilters]);

  const handleCancelSelected = async () => {
    if (selectedJobs.length > 0) {
      await cancelMultipleJobs(selectedJobs);
    }
  };

  const statusCounts = {
    pending: filteredJobs.filter((j) => j.status === 'pending').length,
    processing: filteredJobs.filter((j) => j.status === 'processing').length,
    completed: filteredJobs.filter((j) => j.status === 'completed').length,
    failed: filteredJobs.filter((j) => j.status === 'failed').length,
  };

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">Jobs de Impressão</h1>
          <p className="text-muted-foreground">Gerencie todos os trabalhos de impressão</p>
        </div>
        {selectedJobs.length > 0 && (
          <div className="flex items-center gap-2">
            <span className="text-sm text-muted-foreground">
              {selectedJobs.length} selecionado(s)
            </span>
            <Button variant="destructive" onClick={handleCancelSelected}>
              <X className="mr-2 h-4 w-4" />
              Cancelar Selecionados
            </Button>
            <Button variant="ghost" onClick={clearSelection}>
              Limpar
            </Button>
          </div>
        )}
      </div>

      {/* Status Overview */}
      <div className="flex items-center gap-4">
        <Badge variant="outline" className="px-3 py-1">
          Pendente: {statusCounts.pending}
        </Badge>
        <Badge variant="warning" className="px-3 py-1">
          Processando: {statusCounts.processing}
        </Badge>
        <Badge variant="success" className="px-3 py-1">
          Concluído: {statusCounts.completed}
        </Badge>
        <Badge variant="destructive" className="px-3 py-1">
          Falhou: {statusCounts.failed}
        </Badge>
      </div>

      {/* Filters */}
      <div className="flex items-center gap-4">
        <div className="relative flex-1 max-w-md">
          <Search className="absolute left-3 top-1/2 h-4 w-4 -translate-y-1/2 text-muted-foreground" />
          <Input
            placeholder="Buscar jobs..."
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            className="pl-10"
          />
        </div>
        <Button variant="outline">
          <Filter className="mr-2 h-4 w-4" />
          Filtros
        </Button>
      </div>

      {/* Jobs List */}
      {loading ? (
        <div className="text-center py-12">
          <p className="text-muted-foreground">Carregando jobs...</p>
        </div>
      ) : filteredJobs.length === 0 ? (
        <div className="text-center py-12">
          <p className="text-muted-foreground">Nenhum job encontrado</p>
        </div>
      ) : (
        <div className="space-y-3">
          {filteredJobs.map((job) => (
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
  );
};

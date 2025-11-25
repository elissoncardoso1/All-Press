import { useMemo } from 'react';
import { usePrinterStore } from '../store/printerStore';

export const useFilteredPrinters = () => {
  const printers = usePrinterStore((state) => state.printers);
  const filters = usePrinterStore((state) => state.filters);

  return useMemo(() => {
    let filtered = [...printers];

    // Filter by status
    if (filters.status && filters.status.length > 0) {
      filtered = filtered.filter((p) => filters.status!.includes(p.status));
    }

    // Filter by type
    if (filters.type && filters.type.length > 0) {
      filtered = filtered.filter((p) => filters.type!.includes(p.type));
    }

    // Filter by search
    if (filters.search) {
      const search = filters.search.toLowerCase();
      filtered = filtered.filter(
        (p) =>
          p.name.toLowerCase().includes(search) ||
          p.model.toLowerCase().includes(search) ||
          p.manufacturer.toLowerCase().includes(search) ||
          p.location?.toLowerCase().includes(search)
      );
    }

    return filtered;
  }, [printers, filters]);
};

import { useMemo } from 'react';
import { useJobStore } from '../store/jobStore';

export const useFilteredJobs = () => {
  const jobs = useJobStore((state) => state.jobs);
  const filters = useJobStore((state) => state.filters);

  return useMemo(() => {
    let filtered = [...jobs];

    // Filter by status
    if (filters.status && filters.status.length > 0) {
      filtered = filtered.filter((j) => filters.status!.includes(j.status));
    }

    // Filter by printer
    if (filters.printer && filters.printer.length > 0) {
      filtered = filtered.filter((j) => filters.printer!.includes(j.printerId));
    }

    // Filter by date range
    if (filters.dateRange) {
      const [start, end] = filters.dateRange;
      filtered = filtered.filter((j) => {
        const jobDate = new Date(j.createdAt);
        return jobDate >= start && jobDate <= end;
      });
    }

    // Filter by search
    if (filters.search) {
      const search = filters.search.toLowerCase();
      filtered = filtered.filter(
        (j) =>
          j.fileName.toLowerCase().includes(search) ||
          j.printerName.toLowerCase().includes(search) ||
          j.user?.toLowerCase().includes(search)
      );
    }

    return filtered;
  }, [jobs, filters]);
};

import { create } from 'zustand';
import type { PrintJob, JobFilters } from '../types';
import { jobAPI } from '../services/api';

interface JobState {
  jobs: PrintJob[];
  selectedJobs: string[];
  filters: JobFilters;
  loading: boolean;
  error: string | null;

  // Actions
  fetchJobs: () => Promise<void>;
  cancelJob: (id: string) => Promise<void>;
  retryJob: (id: string) => Promise<void>;  // 🆕 Retry job
  cancelMultipleJobs: (ids: string[]) => Promise<void>;
  selectJob: (id: string) => void;
  deselectJob: (id: string) => void;
  selectAllJobs: () => void;
  clearSelection: () => void;
  setFilters: (filters: JobFilters) => void;
  updateJob: (job: PrintJob) => void;
}

export const useJobStore = create<JobState>((set, get) => ({
  jobs: [],
  selectedJobs: [],
  filters: {},
  loading: false,
  error: null,

  fetchJobs: async () => {
    set({ loading: true, error: null });
    try {
      const jobs = await jobAPI.getAll();
      set({ jobs, loading: false });
    } catch (error) {
      set({ error: 'Falha ao carregar jobs', loading: false });
      console.error('Error fetching jobs:', error);
    }
  },

  cancelJob: async (id) => {
    try {
      await jobAPI.cancel(id);
      const jobs = get().jobs.map((j) =>
        j.id === id ? { ...j, status: 'cancelled' as const } : j
      );
      set({ jobs });
    } catch (error) {
      console.error('Error cancelling job:', error);
    }
  },

  retryJob: async (id) => {
    try {
      await jobAPI.retry(id);
      // Atualizar status para pending
      const jobs = get().jobs.map((j) =>
        j.id === id ? { ...j, status: 'pending' as const, progress: 0 } : j
      );
      set({ jobs });
    } catch (error) {
      console.error('Error retrying job:', error);
      throw error;
    }
  },

  cancelMultipleJobs: async (ids) => {
    try {
      await jobAPI.cancelMultiple(ids);
      const jobs = get().jobs.map((j) =>
        ids.includes(j.id) ? { ...j, status: 'cancelled' as const } : j
      );
      set({ jobs, selectedJobs: [] });
    } catch (error) {
      console.error('Error cancelling jobs:', error);
    }
  },

  selectJob: (id) => {
    const selectedJobs = get().selectedJobs;
    if (!selectedJobs.includes(id)) {
      set({ selectedJobs: [...selectedJobs, id] });
    }
  },

  deselectJob: (id) => {
    set({ selectedJobs: get().selectedJobs.filter((jobId) => jobId !== id) });
  },

  selectAllJobs: () => {
    set({ selectedJobs: get().jobs.map((j) => j.id) });
  },

  clearSelection: () => {
    set({ selectedJobs: [] });
  },

  setFilters: (filters) => {
    set({ filters });
  },

  updateJob: (job) => {
    const jobs = get().jobs.map((j) => (j.id === job.id ? job : j));
    if (!jobs.find((j) => j.id === job.id)) {
      jobs.unshift(job);
    }
    set({ jobs });
  },
}));

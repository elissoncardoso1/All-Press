import axios from 'axios';
import type { Printer, PrintJob, PrintOptions, SystemMetrics, SystemStatus, DashboardStats } from '@/types';

const api = axios.create({
  baseURL: '/api',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json',
  },
});

// Add request interceptor for debugging
api.interceptors.request.use(
  (config) => {
    console.log(`[API] ${config.method?.toUpperCase()} ${config.url}`);
    return config;
  },
  (error) => {
    console.error('[API] Request error:', error);
    return Promise.reject(error);
  }
);

// Add response interceptor for error handling
api.interceptors.response.use(
  (response) => {
    return response;
  },
  (error) => {
    if (error.code === 'ECONNREFUSED' || error.message.includes('Network Error')) {
      console.error('[API] Backend não está acessível. Verifique se o servidor está rodando na porta 8000.');
    }
    return Promise.reject(error);
  }
);

// Printer API
export const printerAPI = {
  getAll: async (): Promise<Printer[]> => {
    const response = await api.get('/printers');
    return response.data;
  },

  getById: async (id: string): Promise<Printer> => {
    const response = await api.get(`/printers/${id}`);
    return response.data;
  },

  discover: async (): Promise<Printer[]> => {
    const response = await api.post('/printers/discover');
    return response.data;
  },

  add: async (uri: string, name?: string): Promise<Printer> => {
    const response = await api.post('/printers', { uri, name });
    return response.data;
  },

  remove: async (id: string): Promise<void> => {
    await api.delete(`/printers/${id}`);
  },

  pause: async (id: string): Promise<void> => {
    await api.post(`/printers/${id}/pause`);
  },

  resume: async (id: string): Promise<void> => {
    await api.post(`/printers/${id}/resume`);
  },

  getJobs: async (id: string): Promise<PrintJob[]> => {
    const response = await api.get(`/printers/${id}/jobs`);
    return response.data;
  },
};

// Job API
export const jobAPI = {
  getAll: async (): Promise<PrintJob[]> => {
    const response = await api.get('/jobs');
    return response.data;
  },

  getById: async (id: string): Promise<PrintJob> => {
    const response = await api.get(`/jobs/${id}`);
    return response.data;
  },

  create: async (printerId: string, file: File, options: PrintOptions): Promise<PrintJob> => {
    const formData = new FormData();
    formData.append('file', file);
    formData.append('printer_id', printerId);
    formData.append('options', JSON.stringify(options));

    const response = await api.post('/jobs', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    return response.data;
  },

  cancel: async (id: string): Promise<void> => {
    await api.post(`/jobs/${id}/cancel`);
  },

  retry: async (id: string): Promise<void> => {
    await api.post(`/jobs/${id}/retry`);
  },

  cancelMultiple: async (ids: string[]): Promise<void> => {
    await api.post('/jobs/cancel-multiple', { job_ids: ids });
  },

  getHistory: async (limit?: number): Promise<PrintJob[]> => {
    const response = await api.get('/jobs/history', {
      params: { limit },
    });
    return response.data;
  },
};

// File API
export const fileAPI = {
  upload: async (file: File, onProgress?: (progress: number) => void): Promise<{ id: string; url: string }> => {
    const formData = new FormData();
    formData.append('file', file);

    const response = await api.post('/files/upload', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
      onUploadProgress: (progressEvent) => {
        if (onProgress && progressEvent.total) {
          const progress = Math.round((progressEvent.loaded * 100) / progressEvent.total);
          onProgress(progress);
        }
      },
    });
    return response.data;
  },

  getPreview: async (fileId: string): Promise<string> => {
    const response = await api.get(`/files/${fileId}/preview`);
    return response.data.url;
  },

  getMetadata: async (fileId: string): Promise<{ pages: number; size: number }> => {
    const response = await api.get(`/files/${fileId}/metadata`);
    return response.data;
  },
};

// System API
export const systemAPI = {
  getMetrics: async (): Promise<SystemMetrics> => {
    const response = await api.get('/system/metrics');
    return response.data;
  },

  getStatus: async (): Promise<SystemStatus> => {
    const response = await api.get('/system/status');
    return response.data;
  },

  getStats: async (): Promise<DashboardStats> => {
    const response = await api.get('/system/stats');
    return response.data;
  },

  getLogs: async (limit?: number): Promise<Array<{ timestamp: Date; level: string; message: string }>> => {
    const response = await api.get('/system/logs', {
      params: { limit },
    });
    return response.data;
  },

  getSettings: async (): Promise<Record<string, any>> => {
    const response = await api.get('/system/settings');
    return response.data;
  },

  saveSettings: async (settings: Record<string, any>): Promise<void> => {
    await api.post('/system/settings', settings);
  },
};

export default api;

// Printer Types
export interface Printer {
  id: string;
  name: string;
  status: 'online' | 'offline' | 'error' | 'busy';
  type: string;
  manufacturer: string;
  model: string;
  location?: string;
  capabilities: PrinterCapabilities;
  currentJobs: number;
  totalJobsProcessed: number;
  lastActivity?: Date;
  ipAddress?: string;
  uri: string;
}

export interface PrinterCapabilities {
  supportedFormats: string[];
  colorSupported: boolean;
  duplexSupported: boolean;
  maxPaperSize: string;
  resolutions: number[];
  paperSizes: string[];
}

// Job Types
export interface PrintJob {
  id: string;
  fileName: string;
  fileSize: number;
  fileType: string;
  status: 'pending' | 'processing' | 'completed' | 'failed' | 'cancelled';
  printerId: string;
  printerName: string;
  progress: number;
  options: PrintOptions;
  createdAt: Date | string;
  startedAt?: Date | string;
  completedAt?: Date | string;
  user?: string;
  estimatedTime?: number;
  currentPage?: number;
  totalPages?: number;
  errorMessage?: string;
}

export interface PrintOptions {
  copies: number;
  paperSize: string;
  orientation: 'portrait' | 'landscape';
  colorMode: 'color' | 'grayscale' | 'monochrome' | 'auto';
  duplex: 'none' | 'short-edge' | 'long-edge';
  quality: 'draft' | 'normal' | 'high';
  pageRange?: string;
  fitToPage?: boolean;
}

// System Types
export interface SystemMetrics {
  cpuUsage: number;
  memoryUsage: number;
  activeConnections: number;
  cacheHitRatio: number;
  threadPoolActive: number;
  threadPoolMax: number;
  requestsPerSecond: number;
  averageResponseTime: number;
}

export interface SystemStatus {
  status: 'online' | 'offline' | 'degraded';
  uptime: number;
  version: string;
  cupsConnected: boolean;
  databaseConnected: boolean;
}

// Stats Types
export interface DashboardStats {
  printersOnline: number;
  printersTotal: number;
  jobsPending: number;
  jobsProcessing: number;
  jobsCompleted: number;
  jobsFailed: number;
  pagesTotal: number;
  pagesToday: number;
  estimatedCost: number;
}

// Notification Types
export interface Notification {
  id: string;
  type: 'info' | 'success' | 'warning' | 'error';
  title: string;
  message: string;
  timestamp: Date;
  read: boolean;
}

// Upload Types
export interface UploadedFile {
  id: string;
  file: File;
  preview?: string;
  status: 'uploading' | 'processing' | 'ready' | 'error';
  progress: number;
  printOptions?: PrintOptions;
  estimatedCost?: number;
  pageCount?: number;
}

// Filter Types
export interface PrinterFilters {
  status?: Printer['status'][];
  type?: string[];
  search?: string;
}

export interface JobFilters {
  status?: PrintJob['status'][];
  printer?: string[];
  dateRange?: [Date, Date];
  search?: string;
}

// WebSocket Message Types
export interface WSMessage {
  type: 'printer_status_update' | 'job_progress_update' | 'system_metrics' | 'notification';
  payload: Printer | PrintJob | SystemMetrics | Notification;
}

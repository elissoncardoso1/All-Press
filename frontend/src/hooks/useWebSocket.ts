import { useEffect } from 'react';
import { wsService } from '../services/websocket';
import { usePrinterStore } from '../store/printerStore';
import { useJobStore } from '../store/jobStore';
import { useSystemStore } from '../store/systemStore';
import type { WSMessage, Printer, PrintJob, SystemMetrics, Notification } from '../types';

export const useWebSocket = () => {
  const updatePrinter = usePrinterStore((state) => state.updatePrinter);
  const updateJob = useJobStore((state) => state.updateJob);
  const updateMetrics = useSystemStore((state) => state.updateMetrics);
  const addNotification = useSystemStore((state) => state.addNotification);

  useEffect(() => {
    // Connect to WebSocket
    wsService.connect();

    // Subscribe to printer status updates
    const unsubPrinter = wsService.on('printer_status_update', (message: WSMessage) => {
      const printer = message.payload as Printer;
      updatePrinter(printer);
    });

    // Subscribe to job progress updates
    const unsubJob = wsService.on('job_progress_update', (message: WSMessage) => {
      const job = message.payload as PrintJob;
      updateJob(job);
    });

    // Subscribe to system metrics
    const unsubMetrics = wsService.on('system_metrics', (message: WSMessage) => {
      const metrics = message.payload as SystemMetrics;
      updateMetrics(metrics);
    });

    // Subscribe to notifications
    const unsubNotification = wsService.on('notification', (message: WSMessage) => {
      const notification = message.payload as Notification;
      addNotification(notification);
    });

    // Cleanup on unmount
    return () => {
      unsubPrinter();
      unsubJob();
      unsubMetrics();
      unsubNotification();
      wsService.disconnect();
    };
  }, [updatePrinter, updateJob, updateMetrics, addNotification]);

  return wsService;
};

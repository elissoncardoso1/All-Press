import { create } from 'zustand';
import type { SystemMetrics, SystemStatus, DashboardStats, Notification } from '../types';
import { systemAPI } from '../services/api';

interface SystemState {
  metrics: SystemMetrics | null;
  status: SystemStatus | null;
  stats: DashboardStats | null;
  notifications: Notification[];
  loading: boolean;
  error: string | null;

  // Actions
  fetchMetrics: () => Promise<void>;
  fetchStatus: () => Promise<void>;
  fetchStats: () => Promise<void>;
  updateMetrics: (metrics: SystemMetrics) => void;
  addNotification: (notification: Omit<Notification, 'id'>) => void;
  markNotificationRead: (id: string) => void;
  clearNotifications: () => void;
}

export const useSystemStore = create<SystemState>((set) => ({
  metrics: null,
  status: null,
  stats: null,
  notifications: [],
  loading: false,
  error: null,

  fetchMetrics: async () => {
    try {
      const metrics = await systemAPI.getMetrics();
      set({ metrics });
    } catch (error) {
      console.error('Error fetching metrics:', error);
    }
  },

  fetchStatus: async () => {
    try {
      const status = await systemAPI.getStatus();
      set({ status });
    } catch (error) {
      console.error('Error fetching status:', error);
      // Set offline status when request fails
      set({ 
        status: { 
          status: 'offline' as const, 
          uptime: 0, 
          version: 'unknown', 
          cupsConnected: false, 
          databaseConnected: false 
        } 
      });
    }
  },

  fetchStats: async () => {
    set({ loading: true, error: null });
    try {
      const stats = await systemAPI.getStats();
      set({ stats, loading: false });
    } catch (error) {
      set({ error: 'Falha ao carregar estatísticas', loading: false });
      console.error('Error fetching stats:', error);
    }
  },

  updateMetrics: (metrics) => {
    set({ metrics });
  },

  addNotification: (notification) => {
    const newNotification: Notification = {
      ...notification,
      id: Date.now().toString(),
      read: false,
    };
    set((state) => ({
      notifications: [newNotification, ...state.notifications].slice(0, 50),
    }));
  },

  markNotificationRead: (id) => {
    set((state) => ({
      notifications: state.notifications.map((n) =>
        n.id === id ? { ...n, read: true } : n
      ),
    }));
  },

  clearNotifications: () => {
    set({ notifications: [] });
  },
}));

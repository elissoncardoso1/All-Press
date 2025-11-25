import { create } from 'zustand';
import type { Printer, PrinterFilters } from '../types';
import { printerAPI } from '../services/api';

interface PrinterState {
  printers: Printer[];
  selectedPrinter: string | null;
  discovering: boolean;
  filters: PrinterFilters;
  loading: boolean;
  error: string | null;

  // Actions
  fetchPrinters: () => Promise<void>;
  discoverPrinters: () => Promise<void>;
  selectPrinter: (id: string | null) => void;
  setFilters: (filters: PrinterFilters) => void;
  pausePrinter: (id: string) => Promise<void>;
  resumePrinter: (id: string) => Promise<void>;
  updatePrinter: (printer: Printer) => void;
}

export const usePrinterStore = create<PrinterState>((set, get) => ({
  printers: [],
  selectedPrinter: null,
  discovering: false,
  filters: {},
  loading: false,
  error: null,

  fetchPrinters: async () => {
    set({ loading: true, error: null });
    try {
      const printers = await printerAPI.getAll();
      console.log('📡 Printers received from API:', printers);
      printers.forEach(p => {
        console.log(`  🖨️  ${p.name}: status=${p.status}, online=${p.status === 'online'}`);
      });
      set({ printers, loading: false });
    } catch (error) {
      set({ error: 'Falha ao carregar impressoras', loading: false });
      console.error('Error fetching printers:', error);
    }
  },

  discoverPrinters: async () => {
    set({ discovering: true, error: null });
    try {
      const printers = await printerAPI.discover();
      set({ printers, discovering: false });
    } catch (error) {
      set({ error: 'Falha ao descobrir impressoras', discovering: false });
      console.error('Error discovering printers:', error);
    }
  },

  selectPrinter: (id) => {
    set({ selectedPrinter: id });
  },

  setFilters: (filters) => {
    set({ filters });
  },

  pausePrinter: async (id) => {
    try {
      await printerAPI.pause(id);
      const printers = get().printers.map((p) =>
        p.id === id ? { ...p, status: 'offline' as const } : p
      );
      set({ printers });
    } catch (error) {
      console.error('Error pausing printer:', error);
    }
  },

  resumePrinter: async (id) => {
    try {
      await printerAPI.resume(id);
      const printers = get().printers.map((p) =>
        p.id === id ? { ...p, status: 'online' as const } : p
      );
      set({ printers });
    } catch (error) {
      console.error('Error resuming printer:', error);
    }
  },

  updatePrinter: (printer) => {
    const printers = get().printers.map((p) => (p.id === printer.id ? printer : p));
    if (!printers.find((p) => p.id === printer.id)) {
      printers.push(printer);
    }
    set({ printers });
  },
}));

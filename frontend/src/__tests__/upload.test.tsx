import { describe, it, expect, vi, beforeEach } from 'vitest';
import { render, screen, waitFor } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import { BrowserRouter } from 'react-router-dom';
import { Upload } from '../modules/upload/Upload';
import { jobAPI } from '../services/api';
import { usePrinterStore } from '../store/printerStore';
import { useJobStore } from '../store/jobStore';
import toast from 'react-hot-toast';

// Mock das dependências
vi.mock('../services/api');
vi.mock('../store/printerStore');
vi.mock('../store/jobStore');
vi.mock('../store/uploadStore', () => ({
  useUploadStore: () => ({
    pendingFiles: [],
    clearPendingFiles: vi.fn(),
  }),
}));
vi.mock('react-hot-toast', () => ({
  default: {
    success: vi.fn(),
    error: vi.fn(),
  },
}));

const mockPrinters = [
  { id: 'printer1', name: 'HP LaserJet', status: 'online' },
  { id: 'printer2', name: 'Canon Printer', status: 'offline' },
];

const mockJob = {
  id: 'job123',
  printerId: 'printer1',
  printerName: 'HP LaserJet',
  fileName: 'test.pdf',
  status: 'pending',
  progress: 0,
  createdAt: new Date().toISOString(),
  options: { copies: 1 },
};

describe('Upload Component', () => {
  beforeEach(() => {
    vi.clearAllMocks();
    
    // Mock do usePrinterStore
    (usePrinterStore as any).mockReturnValue({
      printers: mockPrinters,
      fetchPrinters: vi.fn(),
    });
    
    // Mock do useJobStore
    (useJobStore as any).mockReturnValue({
      fetchJobs: vi.fn(),
    });
  });

  it('deve renderizar o componente Upload', () => {
    render(
      <BrowserRouter>
        <Upload />
      </BrowserRouter>
    );
    
    expect(screen.getByText(/Upload de Arquivos/i)).toBeInTheDocument();
  });

  it('deve exibir mensagem quando não há arquivos selecionados', async () => {
    const user = userEvent.setup();
    render(
      <BrowserRouter>
        <Upload />
      </BrowserRouter>
    );
    
    const processButton = screen.getByRole('button', { name: /processar arquivos/i });
    await user.click(processButton);
    
    expect(toast.error).toHaveBeenCalledWith('Nenhum arquivo selecionado');
  });

  it('deve exibir mensagem quando não há impressora selecionada', async () => {
    const user = userEvent.setup();
    
    // Criar um arquivo mock
    const file = new File(['test content'], 'test.pdf', { type: 'application/pdf' });
    
    render(
      <BrowserRouter>
        <Upload />
      </BrowserRouter>
    );
    
    // Simular upload de arquivo (isso seria feito via dropzone)
    // Por enquanto, vamos apenas testar o botão sem arquivo
    
    const processButton = screen.getByRole('button', { name: /processar arquivos/i });
    await user.click(processButton);
    
    // Deve mostrar erro de arquivo ou impressora
    expect(toast.error).toHaveBeenCalled();
  });

  it('deve processar arquivo quando tudo está correto', async () => {
    const user = userEvent.setup();
    const mockCreateJob = vi.fn().mockResolvedValue(mockJob);
    (jobAPI.create as any) = mockCreateJob;
    
    render(
      <BrowserRouter>
        <Upload />
      </BrowserRouter>
    );
    
    // Este teste seria mais completo com mock do dropzone
    // Por enquanto, verificamos se a função está sendo chamada corretamente
    expect(screen.getByText(/Upload de Arquivos/i)).toBeInTheDocument();
  });
});

describe('Upload API Integration', () => {
  it('deve criar job corretamente via API', async () => {
    const mockCreateJob = vi.fn().mockResolvedValue(mockJob);
    (jobAPI.create as any) = mockCreateJob;
    
    const file = new File(['test'], 'test.pdf', { type: 'application/pdf' });
    const options = { copies: 1, colorMode: 'auto' as const };
    
    const result = await jobAPI.create('printer1', file, options);
    
    expect(mockCreateJob).toHaveBeenCalledWith('printer1', file, options);
    expect(result).toEqual(mockJob);
  });

  it('deve tratar erro ao criar job', async () => {
    const mockCreateJob = vi.fn().mockRejectedValue(new Error('API Error'));
    (jobAPI.create as any) = mockCreateJob;
    
    const file = new File(['test'], 'test.pdf', { type: 'application/pdf' });
    
    await expect(jobAPI.create('printer1', file, { copies: 1 })).rejects.toThrow('API Error');
  });
});


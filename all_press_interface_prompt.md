# Prompt para Interface Completa - All Press C++

## Contexto da Aplicação

Você está desenvolvendo a interface completa para o **All Press C++**, um sistema de gerenciamento de impressão de alta performance para gráficas rápidas. O sistema possui um backend robusto em C++ com APIs nativas e agora precisa de uma interface moderna, intuitiva e responsiva em React/TypeScript.

## Especificações Técnicas

### Stack Frontend
- **Framework:** React 18+ com TypeScript
- **Build Tool:** Vite
- **Styling:** Tailwind CSS + Shadcn/UI
- **Estado:** Zustand ou Redux Toolkit
- **Comunicação:** REST API + WebSocket (tempo real)
- **Icons:** Lucide React
- **Charts:** Recharts ou Chart.js
- **Forms:** React Hook Form + Zod
- **Routing:** React Router v6

### Backend Integration
- **REST API:** C++ HTTP Server (porta 8000)
- **WebSocket:** Servidor nativo C++ (porta 8001)
- **Endpoints principais:** `/api/printers`, `/api/jobs`, `/api/files`
- **Autenticação:** JWT ou session-based

## Estrutura da Interface

### 1. Layout Principal
```
┌─────────────────────────────────────────┐
│ Header: Logo + Navigation + User Menu   │
├─────────────────────────────────────────┤
│ ┌─────────┐ ┌─────────────────────────┐ │
│ │ Sidebar │ │      Main Content       │ │
│ │ Menu    │ │                         │ │
│ │         │ │                         │ │
│ │         │ │                         │ │
│ └─────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────┤
│ Footer: Status + Quick Stats            │
└─────────────────────────────────────────┘
```

### 2. Módulos Principais

#### A. Dashboard (Tela Inicial)
**Componentes necessários:**
- **Cards de estatísticas em tempo real:**
  - Total de impressoras ativas/inativas
  - Jobs em fila/processando/concluídos
  - Páginas impressas hoje/mês
  - Custo estimado do período
- **Gráficos de performance:**
  - Throughput de impressão (jobs/hora)
  - Uso de impressoras por período
  - Distribuição de tipos de arquivo
  - Status de saúde do sistema
- **Lista de impressoras com status visual**
- **Fila de jobs recentes com progress bars**
- **Alertas e notificações**

#### B. Gerenciamento de Impressoras
**Funcionalidades:**
- **Descoberta automática com loading states**
- **Grid/Lista de impressoras com filtros:**
  - Status (online/offline/erro)
  - Tipo (laser/jato/etc)
  - Localização
  - Marca/modelo
- **Card detalhado de cada impressora:**
  - Status em tempo real
  - Capacidades suportadas
  - Jobs em fila
  - Estatísticas de uso
  - Botões de ação (pausar/retomar/configurar)
- **Modal de adição manual de impressora**
- **Configurações avançadas por impressora**

#### C. Gerenciamento de Jobs
**Interface principal:**
- **Tabela/Cards de jobs com filtros avançados:**
  - Status (pending/processing/completed/failed)
  - Data de criação
  - Impressora de destino
  - Tipo de arquivo
  - Usuário
- **Visualizador de fila por impressora**
- **Progress tracking em tempo real**
- **Ações em lote (cancelar/mover/priorizar)**
- **Detalhes expandidos de cada job:**
  - Preview do arquivo
  - Configurações de impressão
  - Log de eventos
  - Tempo estimado

#### D. Upload e Processamento de Arquivos
**Componentes:**
- **Drag & drop area com preview**
- **Upload progress com múltiplos arquivos**
- **Configurador de impressão visual:**
  - Seleção de impressora
  - Opções de papel (A4, A3, etc)
  - Qualidade e modo de cor
  - Duplex e orientação
  - Número de cópias
- **Preview inteligente:**
  - PDFs com thumbnail
  - Imagens com redimensionamento
  - Documentos Office com conversão
- **Estimativa de custo em tempo real**
- **Templates de configuração salvos**

#### E. Monitor de Sistema
**Métricas em tempo real:**
- **Performance do servidor C++:**
  - CPU e RAM usage
  - Throughput de rede
  - Cache hit ratio
  - Thread pool status
- **Status de conectividade:**
  - CUPS connection
  - Network printers
  - Database status
- **Logs do sistema com filtros**
- **Histórico de performance (gráficos)**

#### F. Relatórios e Analytics
**Dashboards analíticos:**
- **Relatórios de uso por período**
- **Análise de custos e consumo**
- **Performance de impressoras**
- **Eficiência operacional**
- **Export para PDF/Excel**

### 3. Componentes de Interface Específicos

#### Real-time Status Components
```typescript
// Componente para status de impressora em tempo real
<PrinterStatusCard 
  printer={printer}
  realTimeUpdates={true}
  showJobs={true}
  actions={['pause', 'resume', 'configure']}
/>

// Progress bar para jobs
<JobProgressBar 
  jobId={job.id}
  progress={job.progress}
  estimatedTime={job.estimated_time}
  realTime={true}
/>

// Monitor de sistema
<SystemHealthMonitor 
  cpuUsage={metrics.cpu}
  memoryUsage={metrics.memory}
  activeConnections={metrics.connections}
  updateInterval={1000}
/>
```

#### File Processing Interface
```typescript
// Upload com preview e configuração
<FileUploadProcessor
  acceptedTypes={['.pdf', '.jpg', '.png', '.docx']}
  maxFiles={10}
  previewEnabled={true}
  autoProcessing={false}
  onUploadComplete={handleUpload}
  printConfigDefaults={defaultPrintOptions}
/>

// Configurador visual de impressão
<PrintOptionsConfigurator
  availablePrinters={printers}
  fileInfo={uploadedFile}
  onChange={handleOptionsChange}
  showCostEstimate={true}
  showPreview={true}
/>
```

### 4. Estados e Gerenciamento de Dados

#### Store Structure (Zustand/Redux)
```typescript
interface AppState {
  // Sistema
  system: {
    status: 'online' | 'offline' | 'degraded';
    performance: SystemMetrics;
    notifications: Notification[];
  };
  
  // Impressoras
  printers: {
    list: Printer[];
    selected: string | null;
    discovering: boolean;
    filters: PrinterFilters;
  };
  
  // Jobs
  jobs: {
    active: PrintJob[];
    history: PrintJob[];
    queue: PrintJob[];
    filters: JobFilters;
    selectedJobs: string[];
  };
  
  // Uploads
  uploads: {
    files: UploadedFile[];
    processing: boolean;
    currentConfig: PrintOptions;
  };
  
  // UI
  ui: {
    sidebarCollapsed: boolean;
    theme: 'light' | 'dark';
    activeModule: string;
    modals: ModalState;
  };
}
```

### 5. Integração WebSocket

#### Real-time Updates
```typescript
// Hook para atualizações em tempo real
const useRealtimeUpdates = () => {
  const [socket, setSocket] = useState<WebSocket | null>(null);
  
  useEffect(() => {
    const ws = new WebSocket('ws://localhost:8001/ws');
    
    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      
      switch (data.type) {
        case 'printer_status_update':
          updatePrinterStatus(data.payload);
          break;
        case 'job_progress_update':
          updateJobProgress(data.payload);
          break;
        case 'system_metrics':
          updateSystemMetrics(data.payload);
          break;
      }
    };
    
    setSocket(ws);
    return () => ws.close();
  }, []);
  
  return socket;
};
```

### 6. Design System e UX

#### Cores e Tema
```css
:root {
  /* Primary - Azul profissional */
  --primary-50: #eff6ff;
  --primary-500: #3b82f6;
  --primary-600: #2563eb;
  --primary-900: #1e3a8a;
  
  /* Status Colors */
  --success: #10b981;
  --warning: #f59e0b;
  --error: #ef4444;
  --info: #06b6d4;
  
  /* Neutral */
  --gray-50: #f9fafb;
  --gray-100: #f3f4f6;
  --gray-900: #111827;
}
```

#### Padrões de Interação
- **Loading states** com skeletons elegantes
- **Feedback visual** imediato para todas as ações
- **Animações suaves** (transitions 200-300ms)
- **Toast notifications** para confirmações
- **Progress indicators** detalhados
- **Empty states** informativos
- **Error boundaries** com recovery

### 7. Performance e Otimizações

#### Estratégias de Otimização
```typescript
// Lazy loading de módulos
const PrinterManagement = lazy(() => import('./modules/PrinterManagement'));
const JobQueue = lazy(() => import('./modules/JobQueue'));
const Reports = lazy(() => import('./modules/Reports'));

// Memoização de componentes pesados
const PrinterGrid = memo(({ printers, filters }) => {
  const filteredPrinters = useMemo(() => 
    filterPrinters(printers, filters), [printers, filters]
  );
  
  return <Grid items={filteredPrinters} />;
});

// Virtualization para listas grandes
const JobList = () => {
  return (
    <FixedSizeList
      height={600}
      itemCount={jobs.length}
      itemSize={80}
      itemData={jobs}
    >
      {JobItem}
    </FixedSizeList>
  );
};
```

### 8. Acessibilidade e Responsive

#### Responsividade
```css
/* Mobile First */
@screen sm { /* 640px+ */ }
@screen md { /* 768px+ */ }
@screen lg { /* 1024px+ */ }
@screen xl { /* 1280px+ */ }
@screen 2xl { /* 1536px+ */ }

/* Layout adaptativo */
.dashboard-grid {
  @apply grid gap-4;
  @apply grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4;
}
```

#### Acessibilidade
- **ARIA labels** em todos os componentes
- **Navegação por teclado** completa
- **Screen reader support**
- **High contrast mode** support
- **Focus management** apropriado
- **Skip navigation** links

### 9. Testing Strategy

#### Tipos de Teste
```typescript
// Unit tests para componentes
describe('PrinterStatusCard', () => {
  it('should display correct status', () => {
    render(<PrinterStatusCard printer={mockPrinter} />);
    expect(screen.getByText('Online')).toBeInTheDocument();
  });
});

// Integration tests para APIs
describe('PrintAPI', () => {
  it('should fetch printers correctly', async () => {
    const printers = await PrintAPI.getPrinters();
    expect(printers).toHaveLength(3);
  });
});

// E2E tests para fluxos principais
describe('Print Job Flow', () => {
  it('should complete full print job submission', () => {
    cy.visit('/upload');
    cy.get('[data-testid="file-drop"]').attachFile('test.pdf');
    cy.get('[data-testid="printer-select"]').select('HP LaserJet');
    cy.get('[data-testid="submit-job"]').click();
    cy.get('[data-testid="job-confirmation"]').should('be.visible');
  });
});
```

### 10. Deployment e Build

#### Build Configuration
```typescript
// vite.config.ts
export default defineConfig({
  plugins: [react(), tsconfigPaths()],
  build: {
    target: 'es2020',
    outDir: 'dist',
    sourcemap: true,
    rollupOptions: {
      output: {
        manualChunks: {
          vendor: ['react', 'react-dom'],
          charts: ['recharts', 'chart.js'],
          forms: ['react-hook-form', 'zod']
        }
      }
    }
  },
  server: {
    proxy: {
      '/api': 'http://localhost:8000',
      '/ws': {
        target: 'ws://localhost:8001',
        ws: true
      }
    }
  }
});
```

## Instruções de Implementação

### Prioridades de Desenvolvimento
1. **Fase 1:** Layout base + Dashboard + Listagem de impressoras
2. **Fase 2:** Upload de arquivos + Configuração de jobs
3. **Fase 3:** Monitor de jobs em tempo real + WebSocket integration
4. **Fase 4:** Relatórios + Analytics + Configurações avançadas
5. **Fase 5:** Polimento + Testes + Otimizações

### Critérios de Qualidade
- **Performance:** Carregamento < 3s, interações < 200ms
- **Usabilidade:** Interface intuitiva para operadores de gráfica
- **Confiabilidade:** Error handling robusto, recovery automático
- **Escalabilidade:** Suporte a centenas de impressoras e milhares de jobs
- **Profissionalismo:** Design clean e funcional para ambiente empresarial

### Entregáveis Esperados
- **Código React/TypeScript** bem estruturado e documentado
- **Storybook** com componentes documentados
- **Testes automatizados** com boa cobertura
- **Documentação** de APIs e componentes
- **Build otimizado** para produção
- **Docker setup** para desenvolvimento e produção

Desenvolva uma interface moderna, robusta e altamente performática que aproveite ao máximo as capacidades do backend C++ de alta performance!

# 🎯 GUIA DE IMPLEMENTAÇÃO - ALL PRESS

## Estrutura de Pasta Recomendada

```
src/
├── components/
│   ├── ui/                          # Componentes base reutilizáveis
│   │   ├── Button.tsx
│   │   ├── Card.tsx
│   │   ├── Badge.tsx
│   │   ├── Input.tsx
│   │   ├── Select.tsx
│   │   ├── Dialog.tsx
│   │   ├── Alert.tsx
│   │   ├── Loader.tsx
│   │   └── Skeleton.tsx
│   │
│   ├── status/                      # Componentes de status
│   │   ├── StatusIndicator.tsx
│   │   ├── PrinterStatus.tsx
│   │   └── JobStatus.tsx
│   │
│   ├── layout/
│   │   ├── MainLayout.tsx
│   │   ├── Sidebar.tsx
│   │   ├── TopBar.tsx
│   │   └── Footer.tsx
│   │
│   └── common/                      # Componentes específicos do domínio
│       ├── PrinterCard.tsx
│       ├── JobCard.tsx
│       ├── PrinterTable.tsx
│       └── JobTable.tsx
│
├── modules/
│   ├── dashboard/
│   │   ├── Dashboard.tsx
│   │   ├── components/
│   │   └── hooks/
│   │
│   ├── printers/
│   │   ├── Printers.tsx
│   │   ├── PrinterList.tsx
│   │   ├── PrinterDetail.tsx
│   │   └── hooks/
│   │
│   ├── jobs/
│   │   ├── Jobs.tsx
│   │   ├── JobList.tsx
│   │   ├── JobDetail.tsx
│   │   └── hooks/
│   │
│   ├── upload/
│   │   ├── Upload.tsx
│   │   ├── UploadForm.tsx
│   │   └── hooks/
│   │
│   └── monitor/
│       ├── Monitor.tsx
│       ├── RealTimeChart.tsx
│       └── hooks/
│
├── hooks/
│   ├── useWebSocket.ts
│   ├── useMobileMenu.ts
│   └── useNotification.ts
│
├── styles/
│   ├── index.css              # Design system
│   └── animations.css         # Animações customizadas
│
└── App.tsx
```

---

## 1️⃣ IMPLEMENTAÇÃO DO DESIGN SYSTEM

### Passo 1: Atualizar `index.css`

Substituir arquivo atual com a versão otimizada (fornecida em outputs/index.css).

**Inclui:**
- ✅ Variáveis CSS organizadas
- ✅ Sistema de spacing (8px base)
- ✅ Componentes CSS reutilizáveis
- ✅ Estados de impressoras
- ✅ Animações profissionais
- ✅ Suporte a acessibilidade

---

## 2️⃣ CRIAR COMPONENTES BASE

### Estrutura de Pastas

```
src/components/ui/
├── Button.tsx          # 200 linhas
├── Card.tsx            # 150 linhas
├── Badge.tsx           # 100 linhas
├── Input.tsx           # 120 linhas
├── Select.tsx          # 130 linhas
├── Dialog.tsx          # 150 linhas
├── Alert.tsx           # 100 linhas
├── Loader.tsx          # 50 linhas
├── Skeleton.tsx        # 60 linhas
└── index.ts            # Exportações
```

### `components/ui/index.ts`

```typescript
export { Button } from './Button';
export { Card, CardHeader, CardContent, CardFooter } from './Card';
export { Badge } from './Badge';
export { Input } from './Input';
export { Select } from './Select';
export { Dialog, DialogContent, DialogHeader, DialogFooter } from './Dialog';
export { Alert } from './Alert';
export { Loader } from './Loader';
export { Skeleton } from './Skeleton';
```

---

## 3️⃣ COMPONENTES DE STATUS

### `components/status/StatusIndicator.tsx`

```tsx
interface StatusIndicatorProps {
  status: 'active' | 'idle' | 'queued' | 'error' | 'offline';
  label?: string;
  showLabel?: boolean;
}

export const StatusIndicator: React.FC<StatusIndicatorProps> = ({
  status,
  label,
  showLabel = true,
}) => {
  const config = {
    active: {
      color: 'bg-success',
      label: 'Ativo',
      animated: true,
    },
    idle: {
      color: 'bg-muted-foreground',
      label: 'Inativo',
      animated: false,
    },
    queued: {
      color: 'bg-warning',
      label: 'Na Fila',
      animated: true,
    },
    error: {
      color: 'bg-destructive',
      label: 'Erro',
      animated: true,
    },
    offline: {
      color: 'bg-muted',
      label: 'Offline',
      animated: false,
    },
  };

  const current = config[status];

  return (
    <div className="flex items-center gap-2">
      <div
        className={`
          w-3 h-3 rounded-full ${current.color}
          ${current.animated ? 'animate-pulse-subtle' : ''}
        `}
      />
      {showLabel && (
        <span className="text-sm font-medium text-muted-foreground">
          {label || current.label}
        </span>
      )}
    </div>
  );
};
```

---

## 4️⃣ EXEMPLO DE PÁGINA REFATORADA

### `modules/dashboard/Dashboard.tsx`

```tsx
import { useQuery } from '@tanstack/react-query';
import {
  Card,
  CardHeader,
  CardContent,
  Badge,
  Loader,
  Skeleton,
} from '@/components/ui';
import { StatusIndicator } from '@/components/status/StatusIndicator';

interface DashboardStats {
  activePrinters: number;
  totalPrinters: number;
  pendingJobs: number;
  completedToday: number;
  errorRate: number;
  printers: Printer[];
}

export const Dashboard: React.FC = () => {
  const { data: stats, isLoading } = useQuery({
    queryKey: ['dashboard-stats'],
    queryFn: () => fetch('/api/dashboard/stats').then(r => r.json()),
  });

  if (isLoading) {
    return (
      <div className="space-y-6">
        <Skeleton className="h-10 w-32" />
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
          {Array.from({ length: 4 }).map((_, i) => (
            <Skeleton key={i} className="h-24" />
          ))}
        </div>
      </div>
    );
  }

  return (
    <div>
      {/* Header */}
      <div className="section-header">
        <h1 className="section-title">Dashboard</h1>
        <p className="section-subtitle">
          Visão geral do sistema de impressão em tempo real
        </p>
      </div>

      {/* KPI Cards */}
      <div className="grid-auto mb-8">
        <Card>
          <CardHeader>
            <h3 className="text-sm font-medium text-muted-foreground">
              Impressoras Ativas
            </h3>
          </CardHeader>
          <CardContent>
            <div className="flex items-end justify-between">
              <div>
                <p className="text-3xl font-bold text-primary">
                  {stats?.activePrinters}
                </p>
                <p className="text-xs text-muted-foreground mt-1">
                  de {stats?.totalPrinters}
                </p>
              </div>
              <StatusIndicator status="active" showLabel={false} />
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <h3 className="text-sm font-medium text-muted-foreground">
              Jobs Pendentes
            </h3>
          </CardHeader>
          <CardContent>
            <div className="flex items-end justify-between">
              <div>
                <p className="text-3xl font-bold text-warning">
                  {stats?.pendingJobs}
                </p>
                <p className="text-xs text-muted-foreground mt-1">
                  aguardando processamento
                </p>
              </div>
              <StatusIndicator status="queued" showLabel={false} />
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <h3 className="text-sm font-medium text-muted-foreground">
              Processados Hoje
            </h3>
          </CardHeader>
          <CardContent>
            <div className="flex items-end justify-between">
              <div>
                <p className="text-3xl font-bold text-success">
                  {stats?.completedToday}
                </p>
                <p className="text-xs text-muted-foreground mt-1">
                  documentos
                </p>
              </div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <h3 className="text-sm font-medium text-muted-foreground">
              Taxa de Erro
            </h3>
          </CardHeader>
          <CardContent>
            <div className="flex items-end justify-between">
              <div>
                <p className="text-3xl font-bold text-destructive">
                  {stats?.errorRate}%
                </p>
                <p className="text-xs text-muted-foreground mt-1">
                  últimas 24h
                </p>
              </div>
              {stats?.errorRate > 5 && (
                <Badge variant="error" size="sm">Atenção</Badge>
              )}
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Printers Table */}
      <Card>
        <CardHeader>
          <h3 className="text-lg font-semibold">Status das Impressoras</h3>
        </CardHeader>
        <CardContent>
          <div className="overflow-x-auto">
            <table className="w-full">
              <thead>
                <tr className="border-b border-border">
                  <th className="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">
                    Nome
                  </th>
                  <th className="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">
                    Status
                  </th>
                  <th className="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">
                    Jobs
                  </th>
                  <th className="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">
                    Papel
                  </th>
                  <th className="text-right py-3 px-4 text-sm font-semibold text-muted-foreground">
                    Ações
                  </th>
                </tr>
              </thead>
              <tbody>
                {stats?.printers?.map((printer) => (
                  <tr
                    key={printer.id}
                    className="border-b border-border hover:bg-muted/30 transition-colors"
                  >
                    <td className="py-3 px-4 text-sm font-medium">
                      {printer.name}
                    </td>
                    <td className="py-3 px-4">
                      <StatusIndicator status={printer.status} />
                    </td>
                    <td className="py-3 px-4">
                      <Badge
                        variant={
                          printer.status === 'active' ? 'success' : 'default'
                        }
                        size="sm"
                      >
                        {printer.jobCount}
                      </Badge>
                    </td>
                    <td className="py-3 px-4 text-sm">
                      <div className="w-16 h-2 bg-muted rounded-full overflow-hidden">
                        <div
                          className="h-full bg-primary transition-all"
                          style={{
                            width: `${printer.paperLevel}%`,
                          }}
                        />
                      </div>
                    </td>
                    <td className="py-3 px-4 text-right">
                      <a
                        href={`/printers/${printer.id}`}
                        className="text-sm text-primary hover:underline"
                      >
                        Detalhes
                      </a>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </CardContent>
      </Card>
    </div>
  );
};
```

---

## 5️⃣ TAILWIND CONFIG

### `tailwind.config.js`

```js
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      fontFamily: {
        sans: [
          '-apple-system',
          'BlinkMacSystemFont',
          '"Segoe UI"',
          'Roboto',
          '"Helvetica Neue"',
          'Arial',
          'sans-serif',
        ],
        mono: [
          'Fira Code',
          'JetBrains Mono',
          'Menlo',
          'monospace',
        ],
      },
      colors: {
        success: 'hsl(var(--success) / <alpha-value>)',
        warning: 'hsl(var(--warning) / <alpha-value>)',
      },
      keyframes: {
        'pulse-subtle': {
          '0%, 100%': { opacity: '1' },
          '50%': { opacity: '0.8' },
        },
        'slide-in': {
          'from': { opacity: '0', transform: 'translateY(10px)' },
          'to': { opacity: '1', transform: 'translateY(0)' },
        },
      },
      animation: {
        'pulse-subtle': 'pulse-subtle 2s cubic-bezier(0.4, 0, 0.6, 1) infinite',
        'slide-in': 'slide-in 0.25s ease-out',
      },
    },
  },
  plugins: [],
};
```

---

## 6️⃣ CHECKLIST DE IMPLEMENTAÇÃO

- [ ] Copiar novo `index.css` para projeto
- [ ] Criar pasta `components/ui/`
- [ ] Implementar todos os componentes base
- [ ] Criar `components/status/`
- [ ] Refatorar Dashboard
- [ ] Refatorar Printers
- [ ] Refatorar Jobs
- [ ] Testar responsividade mobile
- [ ] Testar modo dark
- [ ] Testar acessibilidade (WCAG AA)
- [ ] Atualizar documentação
- [ ] Deploy e feedback

---

## 7️⃣ PADRÕES DE USO

### Button

```tsx
// Variações
<Button variant="primary">Enviar</Button>
<Button variant="secondary">Cancelar</Button>
<Button variant="destructive">Deletar</Button>
<Button variant="outline">Outline</Button>
<Button variant="ghost">Ghost</Button>

// Tamanhos
<Button size="sm">Pequeno</Button>
<Button size="md">Médio</Button>
<Button size="lg">Grande</Button>

// Estados
<Button disabled>Desabilitado</Button>
<Button isLoading>Carregando...</Button>
```

### Card

```tsx
<Card>
  <CardHeader>
    <h3>Título</h3>
  </CardHeader>
  <CardContent>
    Conteúdo da card
  </CardContent>
  <CardFooter>
    <Button>Ação</Button>
  </CardFooter>
</Card>
```

### Badge

```tsx
<Badge variant="success">Sucesso</Badge>
<Badge variant="warning">Aviso</Badge>
<Badge variant="error">Erro</Badge>
```

---

## 8️⃣ PRÓXIMAS FASES

### Fase 2: Melhorias de UX
- Skeleton loaders
- Empty states
- Error boundaries
- Toast notifications melhoradas

### Fase 3: Performance
- Code splitting
- Lazy loading de rotas
- Image optimization
- Caching strategies

### Fase 4: Analytics
- Tracking de eventos
- Performance monitoring
- Error logging
- User analytics

---

## 📚 Recursos Adicionais

- **Tailwind CSS**: https://tailwindcss.com/docs
- **React Hook Form**: Para formulários
- **TanStack Query**: Para cache de dados
- **Zustand**: Para gerenciamento de estado
- **Framer Motion**: Para animações avançadas


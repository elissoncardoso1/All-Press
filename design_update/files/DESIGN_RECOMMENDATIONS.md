# 🎨 DESIGN SYSTEM - ALL PRESS

## 📋 Recomendações de Melhoria da Aplicação

### 1. ESTRUTURA DE COMPONENTES (Criar estes arquivos)

#### `components/ui/Button.tsx`
```tsx
interface ButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: 'primary' | 'secondary' | 'destructive' | 'outline' | 'ghost';
  size?: 'sm' | 'md' | 'lg';
  isLoading?: boolean;
}

export function Button({ 
  variant = 'primary', 
  size = 'md', 
  isLoading, 
  children, 
  disabled,
  ...props 
}: ButtonProps) {
  const variantStyles = {
    primary: 'bg-primary text-primary-foreground hover:bg-primary/90',
    secondary: 'bg-secondary text-secondary-foreground hover:bg-secondary/90',
    destructive: 'bg-destructive text-destructive-foreground hover:bg-destructive/90',
    outline: 'border border-border bg-background hover:bg-muted',
    ghost: 'hover:bg-muted text-foreground',
  };

  const sizeStyles = {
    sm: 'px-3 py-1.5 text-sm',
    md: 'px-4 py-2 text-base',
    lg: 'px-6 py-3 text-lg',
  };

  return (
    <button
      disabled={disabled || isLoading}
      className={`
        font-medium rounded-lg transition-all duration-base
        disabled:opacity-50 disabled:cursor-not-allowed
        focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-offset-2
        ${variantStyles[variant]}
        ${sizeStyles[size]}
      `}
      {...props}
    >
      {isLoading ? (
        <span className="flex items-center gap-2">
          <span className="w-4 h-4 border-2 border-current border-t-transparent rounded-full animate-spin" />
          Carregando...
        </span>
      ) : children}
    </button>
  );
}
```

#### `components/ui/Card.tsx`
```tsx
export function Card({ children, className = '', ...props }) {
  return (
    <div className={`card-lg ${className}`} {...props}>
      {children}
    </div>
  );
}

export function CardHeader({ children, className = '' }) {
  return <div className={`pb-4 border-b border-border ${className}`}>{children}</div>;
}

export function CardContent({ children, className = '' }) {
  return <div className={`py-4 ${className}`}>{children}</div>;
}

export function CardFooter({ children, className = '' }) {
  return <div className={`pt-4 border-t border-border flex gap-2 justify-end ${className}`}>{children}</div>;
}
```

#### `components/ui/Badge.tsx`
```tsx
interface BadgeProps {
  variant?: 'default' | 'success' | 'warning' | 'error' | 'info';
  children: React.ReactNode;
}

export function Badge({ variant = 'default', children }: BadgeProps) {
  const styles = {
    default: 'bg-primary/10 text-primary border border-primary/20',
    success: 'bg-success/10 text-success border border-success/20',
    warning: 'bg-warning/10 text-warning border border-warning/20',
    error: 'bg-destructive/10 text-destructive border border-destructive/20',
    info: 'bg-accent/10 text-accent border border-accent/20',
  };

  return (
    <span className={`status-badge ${styles[variant]}`}>
      {children}
    </span>
  );
}
```

#### `components/StatusIndicator.tsx`
```tsx
interface StatusIndicatorProps {
  status: 'active' | 'idle' | 'queued' | 'error' | 'offline';
  label?: string;
}

export function StatusIndicator({ status, label }: StatusIndicatorProps) {
  const dotClasses = {
    active: 'status-dot-active',
    idle: 'status-dot-idle',
    queued: 'status-dot-queued',
    error: 'status-dot-error',
    offline: 'status-dot-idle',
  };

  const labels = {
    active: 'Ativo',
    idle: 'Inativo',
    queued: 'Na Fila',
    error: 'Erro',
    offline: 'Offline',
  };

  return (
    <div className="flex items-center gap-2">
      <div className={dotClasses[status]} />
      <span className="text-sm text-muted-foreground">
        {label || labels[status]}
      </span>
    </div>
  );
}
```

---

### 2. LAYOUT - `components/layout/MainLayout.tsx`

```tsx
import { Outlet } from 'react-router-dom';
import { Sidebar } from './Sidebar';
import { TopBar } from './TopBar';
import { useMobileMenu } from '../../hooks/useMobileMenu';

export function MainLayout() {
  const { isOpen, toggle, close } = useMobileMenu();

  return (
    <div className="min-h-screen bg-background">
      {/* Top Bar */}
      <TopBar onMenuToggle={toggle} />
      
      <div className="flex">
        {/* Sidebar */}
        <Sidebar isOpen={isOpen} onClose={close} />
        
        {/* Main Content */}
        <main className="flex-1 overflow-auto">
          <div className="container-page section">
            <Outlet />
          </div>
        </main>
      </div>
    </div>
  );
}
```

---

### 3. DASHBOARD - Exemplo de Página Refatorada

```tsx
import { Card, CardHeader, CardContent } from '@/components/ui/Card';
import { Badge } from '@/components/ui/Badge';
import { StatusIndicator } from '@/components/StatusIndicator';
import { useQuery } from '@tanstack/react-query';

export function Dashboard() {
  const { data: stats } = useQuery({
    queryKey: ['dashboard-stats'],
    queryFn: () => fetch('/api/stats').then(r => r.json()),
  });

  return (
    <div>
      <div className="section-header">
        <h1 className="section-title">Dashboard</h1>
        <p className="section-subtitle">
          Visão geral do sistema de impressão
        </p>
      </div>

      {/* KPI Cards */}
      <div className="grid-auto mb-8">
        <Card>
          <CardHeader>
            <h3 className="text-lg font-semibold">Impressoras Ativas</h3>
          </CardHeader>
          <CardContent>
            <p className="text-4xl font-bold text-primary">
              {stats?.activePrinters || 0}
            </p>
            <p className="text-sm text-muted-foreground mt-2">
              De {stats?.totalPrinters || 0} disponíveis
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <h3 className="text-lg font-semibold">Jobs Pendentes</h3>
          </CardHeader>
          <CardContent>
            <p className="text-4xl font-bold text-warning">
              {stats?.pendingJobs || 0}
            </p>
            <p className="text-sm text-muted-foreground mt-2">
              Aguardando processamento
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <h3 className="text-lg font-semibold">Impressões Hoje</h3>
          </CardHeader>
          <CardContent>
            <p className="text-4xl font-bold text-success">
              {stats?.todayPrints || 0}
            </p>
            <p className="text-sm text-muted-foreground mt-2">
              Documentos processados
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <h3 className="text-lg font-semibold">Taxa de Erro</h3>
          </CardHeader>
          <CardContent>
            <p className="text-4xl font-bold text-destructive">
              {stats?.errorRate || 0}%
            </p>
            <p className="text-sm text-muted-foreground mt-2">
              Últimas 24 horas
            </p>
          </CardContent>
        </Card>
      </div>

      {/* Printer Status Table */}
      <Card>
        <CardHeader>
          <h3 className="text-lg font-semibold">Status das Impressoras</h3>
        </CardHeader>
        <CardContent>
          <div className="overflow-x-auto">
            <table className="w-full text-sm">
              <thead>
                <tr className="border-b border-border">
                  <th className="text-left py-3 px-4 font-semibold">Nome</th>
                  <th className="text-left py-3 px-4 font-semibold">Status</th>
                  <th className="text-left py-3 px-4 font-semibold">Jobs</th>
                  <th className="text-left py-3 px-4 font-semibold">Papel</th>
                </tr>
              </thead>
              <tbody>
                {stats?.printers?.map((printer) => (
                  <tr key={printer.id} className="border-b border-border hover:bg-muted/50">
                    <td className="py-3 px-4">{printer.name}</td>
                    <td className="py-3 px-4">
                      <StatusIndicator status={printer.status} />
                    </td>
                    <td className="py-3 px-4">
                      <Badge variant={printer.status === 'active' ? 'success' : 'default'}>
                        {printer.jobCount}
                      </Badge>
                    </td>
                    <td className="py-3 px-4 text-muted-foreground">
                      {printer.paperLevel}%
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
}
```

---

### 4. PROBLEMAS RESOLVIDOS

| Problema | Solução |
|----------|---------|
| CSS desorganizado | Sistema de design consistente com variables |
| Sem componentes reutilizáveis | Componentes base UI criados |
| Cores genéricas | Paleta específica para print shop |
| Sem spacing system | Escala de espaçamento definida |
| Tipografia inconsistente | Hierarquia clara de tipos |
| Sem feedback visual | Badges, indicators e animações |
| Responsividade rudimentar | Grid system e breakpoints melhorados |
| Acessibilidade incompleta | Focus states e ARIA melhorados |

---

### 5. IMPLEMENTAÇÃO (Passo a Passo)

1. **Remover** `App.css` (obsoleto)
2. **Substituir** `index.css` com versão otimizada
3. **Criar** pasta `components/ui/` com componentes base
4. **Criar** `components/StatusIndicator.tsx`
5. **Refatorar** layouts usando novas classes
6. **Atualizar** módulos (Dashboard, Printers, Jobs, etc)

---

### 6. TIPOGRAFIA RECOMENDADA

**Fonte Sans**: System font stack (já implementado)
**Fonte Mono**: Fira Code para dados técnicos

```css
/* Em tailwind.config.ts */
fonts: {
  sans: ['Segoe UI', 'Roboto', 'system-ui'],
  mono: ['Fira Code', 'JetBrains Mono', 'monospace'],
}
```

---

### 7. CORES RECOMENDADAS (Print Shop Professional)

- **Primária**: Blue (#3B82F6) - Confiável, profissional
- **Sucesso**: Green (#10B981) - Impressões OK
- **Aviso**: Amber (#F59E0B) - Jobs na fila
- **Erro**: Red (#EF4444) - Problemas
- **Neutro**: Gray (#6B7280) - Status offline

---

### 8. PRÓXIMAS MELHORIAS

- [ ] Implementar React Query para caching
- [ ] Adicionar skeleton loaders
- [ ] Criar theme switcher (light/dark)
- [ ] Adicionar atalhos de teclado
- [ ] Implementar analytics
- [ ] Adicionar notificações push
- [ ] Criar modo offline
- [ ] Adicionar preferências do usuário


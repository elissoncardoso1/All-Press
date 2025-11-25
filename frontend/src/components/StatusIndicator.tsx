import * as React from 'react';

export interface StatusIndicatorProps {
  status: 'active' | 'idle' | 'queued' | 'error' | 'offline';
  label?: string;
  size?: 'sm' | 'md';
  showLabel?: boolean;
}

export const StatusIndicator: React.FC<StatusIndicatorProps> = ({
  status,
  label,
  size = 'md',
  showLabel = true,
}) => {
  const dotClasses = {
    active: 'w-2 h-2 rounded-full animate-pulse-subtle status-dot-active',
    idle: 'w-2 h-2 rounded-full status-dot-idle',
    queued: 'w-2 h-2 rounded-full animate-pulse-subtle bg-yellow-500',
    error: 'w-2 h-2 rounded-full animate-pulse bg-red-500',
    offline: 'w-2 h-2 rounded-full status-dot-idle',
  };

  const labels = {
    active: 'Ativo',
    idle: 'Inativo',
    queued: 'Na Fila',
    error: 'Erro',
    offline: 'Offline',
  };

  const textSizes = {
    sm: 'text-xs',
    md: 'text-sm',
  };

  return (
    <div className="flex items-center gap-2">
      <div className={dotClasses[status]} />
      {showLabel && (
        <span className={`${textSizes[size]} text-muted-foreground`}>
          {label || labels[status]}
        </span>
      )}
    </div>
  );
};


// components/ui/Button.tsx
import React from 'react';

interface ButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: 'primary' | 'secondary' | 'destructive' | 'outline' | 'ghost';
  size?: 'sm' | 'md' | 'lg';
  isLoading?: boolean;
  children: React.ReactNode;
}

export const Button = React.forwardRef<HTMLButtonElement, ButtonProps>(
  ({ variant = 'primary', size = 'md', isLoading, children, disabled, className, ...props }, ref) => {
    const variantStyles = {
      primary: 'bg-primary text-primary-foreground hover:bg-primary/90 focus-visible:ring-primary',
      secondary: 'bg-secondary text-secondary-foreground hover:bg-secondary/90',
      destructive: 'bg-destructive text-destructive-foreground hover:bg-destructive/90',
      outline: 'border border-border bg-background text-foreground hover:bg-muted',
      ghost: 'text-foreground hover:bg-muted',
    };

    const sizeStyles = {
      sm: 'px-3 py-1.5 text-sm rounded',
      md: 'px-4 py-2 text-base rounded-md',
      lg: 'px-6 py-3 text-lg rounded-lg',
    };

    const baseStyles = 'font-medium transition-all duration-base disabled:opacity-50 disabled:cursor-not-allowed inline-flex items-center justify-center gap-2';

    return (
      <button
        ref={ref}
        disabled={disabled || isLoading}
        className={`${baseStyles} ${variantStyles[variant]} ${sizeStyles[size]} ${className || ''}`}
        {...props}
      >
        {isLoading && (
          <span className="w-4 h-4 border-2 border-current border-t-transparent rounded-full animate-spin" />
        )}
        {children}
      </button>
    );
  }
);

Button.displayName = 'Button';

// components/ui/Card.tsx
interface CardProps extends React.HTMLAttributes<HTMLDivElement> {
  variant?: 'default' | 'outline' | 'elevated';
}

export const Card = React.forwardRef<HTMLDivElement, CardProps>(
  ({ variant = 'default', className, ...props }, ref) => {
    const variants = {
      default: 'card-lg',
      outline: 'card-md border-2 border-border',
      elevated: 'card-lg shadow-lg hover:shadow-xl transition-shadow',
    };

    return <div ref={ref} className={`${variants[variant]} ${className || ''}`} {...props} />;
  }
);

Card.displayName = 'Card';

export const CardHeader = ({ children, className = '', ...props }: React.HTMLAttributes<HTMLDivElement>) => (
  <div className={`pb-4 border-b border-border ${className}`} {...props}>
    {children}
  </div>
);

export const CardContent = ({ children, className = '', ...props }: React.HTMLAttributes<HTMLDivElement>) => (
  <div className={`py-4 ${className}`} {...props}>
    {children}
  </div>
);

export const CardFooter = ({ children, className = '', ...props }: React.HTMLAttributes<HTMLDivElement>) => (
  <div className={`pt-4 border-t border-border flex gap-2 justify-end ${className}`} {...props}>
    {children}
  </div>
);

// components/ui/Badge.tsx
interface BadgeProps extends React.HTMLAttributes<HTMLSpanElement> {
  variant?: 'default' | 'success' | 'warning' | 'error' | 'info';
  size?: 'sm' | 'md';
}

export const Badge = React.forwardRef<HTMLSpanElement, BadgeProps>(
  ({ variant = 'default', size = 'md', className, children, ...props }, ref) => {
    const variantStyles = {
      default: 'bg-primary/10 text-primary border border-primary/20',
      success: 'bg-success/10 text-success border border-success/20',
      warning: 'bg-warning/10 text-warning border border-warning/20',
      error: 'bg-destructive/10 text-destructive border border-destructive/20',
      info: 'bg-accent/10 text-accent border border-accent/20',
    };

    const sizeStyles = {
      sm: 'px-2 py-1 text-xs',
      md: 'px-3 py-1.5 text-sm',
    };

    return (
      <span
        ref={ref}
        className={`inline-flex items-center gap-1.5 rounded-full font-medium transition-colors ${variantStyles[variant]} ${sizeStyles[size]} ${className || ''}`}
        {...props}
      >
        {children}
      </span>
    );
  }
);

Badge.displayName = 'Badge';

// components/ui/Input.tsx
interface InputProps extends React.InputHTMLAttributes<HTMLInputElement> {
  error?: string;
  label?: string;
}

export const Input = React.forwardRef<HTMLInputElement, InputProps>(
  ({ error, label, className, ...props }, ref) => (
    <div className="flex flex-col gap-2">
      {label && <label className="text-sm font-medium">{label}</label>}
      <input
        ref={ref}
        className={`
          px-4 py-2 rounded-md border border-input bg-background
          text-foreground placeholder:text-muted-foreground
          focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring
          disabled:opacity-50 disabled:cursor-not-allowed
          transition-colors duration-base
          ${error ? 'border-destructive focus-visible:ring-destructive' : ''}
          ${className || ''}
        `}
        {...props}
      />
      {error && <span className="text-sm text-destructive">{error}</span>}
    </div>
  )
);

Input.displayName = 'Input';

// components/StatusIndicator.tsx
interface StatusIndicatorProps {
  status: 'active' | 'idle' | 'queued' | 'error' | 'offline';
  label?: string;
  size?: 'sm' | 'md';
}

export const StatusIndicator: React.FC<StatusIndicatorProps> = ({ status, label, size = 'md' }) => {
  const dotClasses = {
    active: 'w-2 h-2 bg-success rounded-full animate-pulse-subtle',
    idle: 'w-2 h-2 bg-muted-foreground rounded-full',
    queued: 'w-2 h-2 bg-warning rounded-full animate-pulse-subtle',
    error: 'w-2 h-2 bg-destructive rounded-full animate-pulse',
    offline: 'w-2 h-2 bg-muted-foreground rounded-full',
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
      <span className={`${textSizes[size]} text-muted-foreground`}>
        {label || labels[status]}
      </span>
    </div>
  );
};

// components/ui/Dialog.tsx
import { ReactNode, useState } from 'react';

interface DialogProps {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  children: ReactNode;
}

export const Dialog: React.FC<DialogProps> = ({ open, onOpenChange, children }) => {
  if (!open) return null;

  return (
    <div className="fixed inset-0 z-50">
      {/* Backdrop */}
      <div
        className="absolute inset-0 bg-black/50"
        onClick={() => onOpenChange(false)}
      />
      {/* Content */}
      <div className="relative z-10 flex items-center justify-center min-h-screen p-4">
        {children}
      </div>
    </div>
  );
};

export const DialogContent = ({ children, className = '' }: { children: ReactNode; className?: string }) => (
  <div className={`bg-card text-card-foreground rounded-lg shadow-xl max-w-md w-full p-6 ${className}`}>
    {children}
  </div>
);

export const DialogHeader = ({ children, className = '' }: { children: ReactNode; className?: string }) => (
  <div className={`mb-4 pb-4 border-b border-border ${className}`}>
    {children}
  </div>
);

export const DialogFooter = ({ children, className = '' }: { children: ReactNode; className?: string }) => (
  <div className={`mt-6 pt-4 border-t border-border flex justify-end gap-2 ${className}`}>
    {children}
  </div>
);

// components/ui/Select.tsx
interface SelectProps extends React.SelectHTMLAttributes<HTMLSelectElement> {
  label?: string;
  error?: string;
  options: Array<{ value: string; label: string }>;
}

export const Select = React.forwardRef<HTMLSelectElement, SelectProps>(
  ({ label, error, options, className, ...props }, ref) => (
    <div className="flex flex-col gap-2">
      {label && <label className="text-sm font-medium">{label}</label>}
      <select
        ref={ref}
        className={`
          px-4 py-2 rounded-md border border-input bg-background
          text-foreground focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring
          disabled:opacity-50 disabled:cursor-not-allowed
          transition-colors duration-base
          ${error ? 'border-destructive focus-visible:ring-destructive' : ''}
          ${className || ''}
        `}
        {...props}
      >
        {options.map(opt => (
          <option key={opt.value} value={opt.value}>
            {opt.label}
          </option>
        ))}
      </select>
      {error && <span className="text-sm text-destructive">{error}</span>}
    </div>
  )
);

Select.displayName = 'Select';

// components/ui/Skeleton.tsx
interface SkeletonProps extends React.HTMLAttributes<HTMLDivElement> {
  className?: string;
}

export const Skeleton = React.forwardRef<HTMLDivElement, SkeletonProps>(
  ({ className, ...props }, ref) => (
    <div
      ref={ref}
      className={`animate-pulse bg-muted rounded ${className}`}
      {...props}
    />
  )
);

Skeleton.displayName = 'Skeleton';

// components/ui/Loader.tsx
export const Loader = ({ size = 'md' }: { size?: 'sm' | 'md' | 'lg' }) => {
  const sizes = {
    sm: 'w-4 h-4',
    md: 'w-8 h-8',
    lg: 'w-12 h-12',
  };

  return (
    <div className={`${sizes[size]} border-2 border-muted border-t-primary rounded-full animate-spin`} />
  );
};

// components/ui/Alert.tsx
interface AlertProps extends React.HTMLAttributes<HTMLDivElement> {
  variant?: 'default' | 'success' | 'warning' | 'error' | 'info';
  children: ReactNode;
}

export const Alert = React.forwardRef<HTMLDivElement, AlertProps>(
  ({ variant = 'default', children, className, ...props }, ref) => {
    const variants = {
      default: 'bg-primary/10 border border-primary/20 text-primary',
      success: 'bg-success/10 border border-success/20 text-success',
      warning: 'bg-warning/10 border border-warning/20 text-warning',
      error: 'bg-destructive/10 border border-destructive/20 text-destructive',
      info: 'bg-accent/10 border border-accent/20 text-accent',
    };

    return (
      <div
        ref={ref}
        className={`p-4 rounded-lg ${variants[variant]} ${className || ''}`}
        {...props}
      >
        {children}
      </div>
    );
  }
);

Alert.displayName = 'Alert';

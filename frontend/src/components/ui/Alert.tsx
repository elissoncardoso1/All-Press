import * as React from 'react';
import { cn } from '@/lib/utils';

export interface AlertProps extends React.HTMLAttributes<HTMLDivElement> {
  variant?: 'default' | 'success' | 'warning' | 'error' | 'info';
  children: React.ReactNode;
}

const Alert = React.forwardRef<HTMLDivElement, AlertProps>(
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
        className={cn('p-4 rounded-lg', variants[variant], className)}
        role="alert"
        {...props}
      >
        {children}
      </div>
    );
  }
);

Alert.displayName = 'Alert';

export { Alert };


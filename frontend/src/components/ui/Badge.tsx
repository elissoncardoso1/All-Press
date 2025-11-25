import * as React from 'react';
import { cn } from '@/lib/utils';

export interface BadgeProps extends React.HTMLAttributes<HTMLSpanElement> {
  variant?: 'default' | 'success' | 'warning' | 'error' | 'info' | 'secondary' | 'destructive' | 'outline';
  size?: 'sm' | 'md';
}

const Badge = React.forwardRef<HTMLSpanElement, BadgeProps>(
  ({ variant = 'default', size = 'md', className, children, ...props }, ref) => {
    const variantStyles = {
      default: 'bg-blue-50 text-blue-700 border border-blue-200',
      success: 'bg-green-50 text-green-700 border border-green-200',
      warning: 'bg-yellow-50 text-yellow-700 border border-yellow-200',
      error: 'bg-red-50 text-red-700 border border-red-200',
      info: 'bg-cyan-50 text-cyan-700 border border-cyan-200',
      secondary: 'bg-gray-100 text-gray-700 hover:bg-gray-200',
      destructive: 'bg-red-100 text-red-700 hover:bg-red-200',
      outline: 'text-gray-700 border border-gray-300',
    };

    const sizeStyles = {
      sm: 'px-2 py-1 text-xs',
      md: 'px-3 py-1.5 text-sm',
    };

    return (
      <span
        ref={ref}
        className={cn(
          'inline-flex items-center gap-1.5 rounded-full font-medium transition-colors',
          variantStyles[variant],
          sizeStyles[size],
          className
        )}
        role="status"
        {...props}
      >
        {children}
      </span>
    );
  }
);

Badge.displayName = 'Badge';

export { Badge };

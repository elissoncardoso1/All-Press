import { memo } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/Card';
import { cn } from '@/lib/utils';
import type { LucideIcon } from 'lucide-react';
import { Loader2 } from 'lucide-react';

interface StatsCardProps {
  title: string;
  value: string | number;
  description?: string;
  icon: LucideIcon;
  trend?: {
    value: number;
    positive: boolean;
  };
  className?: string;
  isLoading?: boolean;
  onClick?: () => void;
}

export const StatsCard = memo(({ 
  title, 
  value, 
  description, 
  icon: Icon, 
  trend, 
  className,
  isLoading = false,
  onClick 
}: StatsCardProps) => {
  const cardId = `stats-card-${title.toLowerCase().replace(/\s+/g, '-')}`;
  
  return (
    <Card 
      className={cn(
        'transition-all duration-200 hover:shadow-md hover:scale-[1.02]',
        onClick && 'cursor-pointer',
        className
      )}
      onClick={onClick}
      role={onClick ? 'button' : undefined}
      tabIndex={onClick ? 0 : undefined}
      onKeyDown={onClick ? (e) => {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          onClick();
        }
      } : undefined}
      aria-label={onClick ? `${title}: ${value}` : undefined}
      id={cardId}
    >
      <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
        <CardTitle 
          className="text-sm font-medium"
          id={`${cardId}-title`}
        >
          {title}
        </CardTitle>
        <div className="rounded-full bg-primary/10 p-2 transition-colors hover:bg-primary/20">
          <Icon 
            className="h-4 w-4 text-primary" 
            aria-hidden="true"
          />
        </div>
      </CardHeader>
      <CardContent>
        {isLoading ? (
          <div className="flex items-center justify-center py-4" aria-live="polite" aria-busy="true">
            <Loader2 className="h-6 w-6 animate-spin text-muted-foreground" />
            <span className="sr-only">Carregando {title}...</span>
          </div>
        ) : (
          <>
            <div 
              className="text-2xl font-bold"
              aria-labelledby={`${cardId}-title`}
            >
              {value}
            </div>
            {description && (
              <p className="text-xs text-muted-foreground mt-1">
                {description}
              </p>
            )}
            {trend && (
              <p 
                className={cn(
                  'text-xs mt-2 flex items-center gap-1',
                  trend.positive ? 'text-success' : 'text-error'
                )}
                role="status"
                aria-live="polite"
              >
                <span aria-hidden="true">
                  {trend.positive ? '↑' : '↓'}
                </span>
                <span>
                  {trend.positive ? '+' : '-'}{Math.abs(trend.value)}% desde o último período
                </span>
              </p>
            )}
          </>
        )}
      </CardContent>
    </Card>
  );
});

StatsCard.displayName = 'StatsCard';

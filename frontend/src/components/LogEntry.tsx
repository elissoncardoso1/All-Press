import React from 'react';
import type { LogEntry } from '@/hooks/useLogs';
import { Info, AlertTriangle, XCircle, Bug } from 'lucide-react';
import { format } from 'date-fns';

interface LogEntryComponentProps {
    log: LogEntry;
}

export const LogEntryComponent: React.FC<LogEntryComponentProps> = ({ log }) => {
    const getIcon = () => {
        switch (log.level) {
            case 'INFO':
                return <Info className="w-4 h-4 text-blue-500" />;
            case 'WARNING':
                return <AlertTriangle className="w-4 h-4 text-yellow-500" />;
            case 'ERROR':
                return <XCircle className="w-4 h-4 text-red-500" />;
            case 'DEBUG':
                return <Bug className="w-4 h-4 text-gray-500" />;
            default:
                return <Info className="w-4 h-4 text-gray-400" />;
        }
    };

    const getBackgroundColor = () => {
        switch (log.level) {
            case 'INFO':
                return 'bg-blue-50/50 hover:bg-blue-50 border-blue-100';
            case 'WARNING':
                return 'bg-yellow-50/50 hover:bg-yellow-50 border-yellow-100';
            case 'ERROR':
                return 'bg-red-50/50 hover:bg-red-50 border-red-100';
            case 'DEBUG':
                return 'bg-gray-50/50 hover:bg-gray-50 border-gray-100';
            default:
                return 'bg-card hover:bg-muted border-border';
        }
    };

    const getLevelColor = () => {
        switch (log.level) {
            case 'INFO':
                return 'text-blue-700 bg-blue-100';
            case 'WARNING':
                return 'text-yellow-700 bg-yellow-100';
            case 'ERROR':
                return 'text-red-700 bg-red-100';
            case 'DEBUG':
                return 'text-gray-700 bg-gray-100';
            default:
                return 'text-gray-700 bg-gray-100';
        }
    };

    const timestamp = typeof log.timestamp === 'string' ? new Date(log.timestamp) : log.timestamp;

    return (
        <div className={`flex items-start gap-3 p-3 rounded-lg border transition-colors ${getBackgroundColor()}`}>
            <div className="mt-0.5">{getIcon()}</div>
            <div className="flex-1 min-w-0">
                <div className="flex items-center gap-2 mb-1">
                    <span className={`px-2 py-0.5 text-xs font-medium rounded ${getLevelColor()}`}>
                        {log.level}
                    </span>
                    <span className="text-xs text-muted-foreground font-mono">
                        {format(timestamp, 'yyyy-MM-dd HH:mm:ss')}
                    </span>
                </div>
                <p className="text-sm text-foreground font-mono break-words">{log.message}</p>
            </div>
        </div>
    );
};

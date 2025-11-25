import React from 'react';
import { Printer, AlertCircle, CheckCircle2, WifiOff } from 'lucide-react';

interface PrinterCardProps {
    name: string;
    status: 'online' | 'offline' | 'warning' | 'error';
    inkLevel: number;
    jobsInQueue: number;
}

export const PrinterCard: React.FC<PrinterCardProps> = ({ name, status, inkLevel, jobsInQueue }) => {
    const getStatusColor = (status: string) => {
        switch (status) {
            case 'online': return 'text-success border-success/20 bg-success/10';
            case 'warning': return 'text-warning border-warning/20 bg-warning/10';
            case 'error': return 'text-destructive border-destructive/20 bg-destructive/10';
            default: return 'text-muted-foreground border-muted/20 bg-muted/10';
        }
    };

    const getStatusIcon = (status: string) => {
        switch (status) {
            case 'online': return <CheckCircle2 size={16} />;
            case 'warning': return <AlertCircle size={16} />;
            case 'error': return <AlertCircle size={16} />;
            default: return <WifiOff size={16} />;
        }
    };

    return (
        <div className="group relative bg-card/80 backdrop-blur-sm border border-border rounded-xl p-5 transition-all duration-300 hover:scale-[1.02] hover:shadow-[0_0_20px_rgba(59,130,246,0.1)] hover:border-primary/30">
            <div className="flex justify-between items-start mb-4">
                <div className="flex items-center gap-3">
                    <div className={`p-2 rounded-lg bg-secondary/50 text-primary group-hover:text-accent transition-colors`}>
                        <Printer size={24} />
                    </div>
                    <div>
                        <h3 className="font-semibold text-lg text-foreground tracking-tight">{name}</h3>
                        <p className="text-xs text-muted-foreground">IP: 192.168.1.105</p>
                    </div>
                </div>
                <div className={`flex items-center gap-1.5 px-2.5 py-1 rounded-full border text-xs font-medium ${getStatusColor(status)}`}>
                    {status === 'online' && <span className="w-1.5 h-1.5 rounded-full bg-current animate-pulse" />}
                    {getStatusIcon(status)}
                    <span className="capitalize">{status}</span>
                </div>
            </div>

            <div className="space-y-4">
                <div className="space-y-1.5">
                    <div className="flex justify-between text-xs">
                        <span className="text-muted-foreground">Ink Level</span>
                        <span className="text-foreground font-medium">{inkLevel}%</span>
                    </div>
                    <div className="h-1.5 w-full bg-secondary rounded-full overflow-hidden">
                        <div
                            className="h-full bg-gradient-to-r from-primary to-accent transition-all duration-500 ease-out"
                            style={{ width: `${inkLevel}%` }}
                        />
                    </div>
                </div>

                <div className="flex items-center justify-between pt-2 border-t border-border/50">
                    <span className="text-xs text-muted-foreground">Queue Status</span>
                    <span className="text-sm font-medium text-foreground bg-secondary/50 px-2 py-0.5 rounded">
                        {jobsInQueue} Jobs
                    </span>
                </div>
            </div>
        </div>
    );
};

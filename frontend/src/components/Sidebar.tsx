import React from 'react';
import { LayoutDashboard, Printer, List, Settings, Activity, Upload, Monitor as MonitorIcon } from 'lucide-react';
import { useNavigate, useLocation } from 'react-router-dom';

export const Sidebar: React.FC = () => {
    const navigate = useNavigate();
    const location = useLocation();

    const navItems = [
        { icon: LayoutDashboard, label: 'Dashboard', path: '/dashboard' },
        { icon: Printer, label: 'Impressoras', path: '/printers' },
        { icon: List, label: 'Fila de Jobs', path: '/jobs' },
        { icon: Upload, label: 'Upload', path: '/upload' },
        { icon: MonitorIcon, label: 'Monitor', path: '/monitor' },
        { icon: Activity, label: 'Logs', path: '/logs' },
        { icon: Settings, label: 'Configurações', path: '/settings' },
    ];

    return (
        <aside className="w-64 h-full bg-card/95 backdrop-blur-sm border-r border-border/50 flex flex-col">
            <div className="p-6 border-b border-border/50">
                <h1 className="text-2xl font-bold bg-gradient-to-r from-primary to-accent bg-clip-text text-transparent">
                    All Press C++
                </h1>
                <p className="text-xs text-muted-foreground mt-1">High-Performance Engine</p>
            </div>

            <nav className="flex-1 p-4 space-y-2">
                {navItems.map((item) => {
                    const isActive = location.pathname === item.path;
                    return (
                        <button
                            key={item.path}
                            onClick={() => navigate(item.path)}
                            className={`w-full flex items-center gap-3 px-4 py-3 rounded-lg transition-all duration-200 group relative overflow-hidden ${isActive
                                    ? 'text-primary-foreground bg-primary/10 border border-primary/20 shadow-[0_0_15px_rgba(59,130,246,0.15)]'
                                    : 'text-muted-foreground hover:text-foreground hover:bg-secondary/50'
                                }`}
                        >
                            {isActive && (
                                <div className="absolute left-0 top-0 bottom-0 w-1 bg-primary shadow-[0_0_10px_#3B82F6]" />
                            )}
                            <item.icon
                                size={20}
                                className={`transition-transform duration-200 ${isActive ? 'text-primary scale-110' : 'group-hover:scale-110'
                                    }`}
                            />
                            <span className="font-medium">{item.label}</span>
                        </button>
                    );
                })}
            </nav>

            <div className="p-4 border-t border-border/50">
                <div className="flex items-center gap-3 px-4 py-3 rounded-lg bg-secondary/30 border border-border/50">
                    <div className="w-2 h-2 rounded-full bg-success animate-pulse" />
                    <div className="flex flex-col">
                        <span className="text-xs font-medium text-foreground">System Online</span>
                        <span className="text-[10px] text-muted-foreground">v1.0.0 Stable</span>
                    </div>
                </div>
            </div>
        </aside>
    );
};

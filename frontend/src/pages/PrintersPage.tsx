import React from 'react';

export const PrintersPage: React.FC = () => {
    return (
        <div className="space-y-6">
            <div className="flex items-center justify-between">
                <div>
                    <h2 className="text-3xl font-bold text-foreground tracking-tight">Impressoras</h2>
                    <p className="text-muted-foreground mt-1">Gerenciamento de dispositivos</p>
                </div>
            </div>
            <div className="p-12 text-center border border-dashed border-border rounded-xl bg-card/50">
                <p className="text-muted-foreground">Lista completa de impressoras será implementada aqui.</p>
            </div>
        </div>
    );
};

import React, { type ReactNode } from 'react';

interface SettingsSectionProps {
    title: string;
    description?: string;
    children: ReactNode;
}

export const SettingsSection: React.FC<SettingsSectionProps> = ({ title, description, children }) => {
    return (
        <div className="bg-card border border-border rounded-lg p-6">
            <div className="mb-4">
                <h3 className="text-lg font-semibold text-foreground">{title}</h3>
                {description && <p className="text-sm text-muted-foreground mt-1">{description}</p>}
            </div>
            <div className="space-y-4">{children}</div>
        </div>
    );
};

interface SettingItemProps {
    label: string;
    description?: string;
    children: ReactNode;
}

export const SettingItem: React.FC<SettingItemProps> = ({ label, description, children }) => {
    return (
        <div className="flex items-center justify-between gap-4">
            <div className="flex-1">
                <label className="text-sm font-medium text-foreground">{label}</label>
                {description && <p className="text-xs text-muted-foreground mt-0.5">{description}</p>}
            </div>
            <div className="w-64">{children}</div>
        </div>
    );
};

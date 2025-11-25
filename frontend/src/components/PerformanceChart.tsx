import React from 'react';
import { LineChart, Line, ResponsiveContainer, Tooltip } from 'recharts';

const data = [
    { time: '10:00', speed: 400 },
    { time: '10:01', speed: 300 },
    { time: '10:02', speed: 550 },
    { time: '10:03', speed: 450 },
    { time: '10:04', speed: 600 },
    { time: '10:05', speed: 500 },
    { time: '10:06', speed: 700 },
];

export const PerformanceChart: React.FC = () => {
    return (
        <div className="h-[200px] w-full min-w-0">
            <ResponsiveContainer width="100%" height="100%" minHeight={200}>
                <LineChart data={data} width={500} height={200}>
                    <defs>
                        <linearGradient id="colorSpeed" x1="0" y1="0" x2="0" y2="1">
                            <stop offset="5%" stopColor="#3B82F6" stopOpacity={0.3} />
                            <stop offset="95%" stopColor="#3B82F6" stopOpacity={0} />
                        </linearGradient>
                    </defs>
                    <Tooltip
                        contentStyle={{
                            backgroundColor: '#1A1E23',
                            border: '1px solid #334155',
                            borderRadius: '8px',
                            color: '#fff'
                        }}
                        itemStyle={{ color: '#3B82F6' }}
                    />
                    <Line
                        type="monotone"
                        dataKey="speed"
                        stroke="#3B82F6"
                        strokeWidth={2}
                        dot={false}
                        activeDot={{ r: 4, fill: '#3B82F6', stroke: '#fff' }}
                    />
                </LineChart>
            </ResponsiveContainer>
        </div>
    );
};

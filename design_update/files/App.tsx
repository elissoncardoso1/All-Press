import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { Toaster } from 'react-hot-toast';
import { MainLayout } from './components/layout/MainLayout';
import { Dashboard } from './modules/dashboard/Dashboard';
import { Printers } from './modules/printers/Printers';
import { Jobs } from './modules/jobs/Jobs';
import { Upload } from './modules/upload/Upload';
import { Monitor } from './modules/monitor/Monitor';
import { useWebSocket } from './hooks/useWebSocket';

function App() {
  // Initialize WebSocket connection
  useWebSocket();

  return (
    <BrowserRouter>
      <Routes>
        <Route element={<MainLayout />}>
          <Route path="/" element={<Dashboard />} />
          <Route path="/printers" element={<Printers />} />
          <Route path="/jobs" element={<Jobs />} />
          <Route path="/upload" element={<Upload />} />
          <Route path="/monitor" element={<Monitor />} />
        </Route>
      </Routes>
      <Toaster 
        position="top-right"
        toastOptions={{
          duration: 4000,
          style: {
            background: 'hsl(var(--card))',
            color: 'hsl(var(--card-foreground))',
            border: '1px solid hsl(var(--border))',
            borderRadius: 'calc(var(--radius) * 2)',
            boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)',
          },
        }}
      />
    </BrowserRouter>
  );
}

export default App;

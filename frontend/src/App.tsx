import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import { Layout } from './components/Layout';
import { Dashboard } from './pages/Dashboard';
import { PrintersPage } from './pages/PrintersPage';
import { JobsPage } from './pages/JobsPage';
import { LogsPage } from './pages/LogsPage';
import { SettingsPage } from './pages/SettingsPage';
import { Upload } from './modules/upload/Upload';
import { Monitor } from './modules/monitor/Monitor';

function App() {
  return (
    <BrowserRouter>
      <Layout>
        <Routes>
          <Route path="/" element={<Navigate to="/dashboard" replace />} />
          <Route path="/dashboard" element={<Dashboard />} />
          <Route path="/printers" element={<PrintersPage />} />
          <Route path="/jobs" element={<JobsPage />} />
          <Route path="/logs" element={<LogsPage />} />
          <Route path="/settings" element={<SettingsPage />} />
          <Route path="/upload" element={<Upload />} />
          <Route path="/monitor" element={<Monitor />} />
        </Routes>
      </Layout>
    </BrowserRouter>
  );
}

export default App;

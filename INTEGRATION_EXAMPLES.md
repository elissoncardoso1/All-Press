# All Press C++ - Exemplos de Integração

## 1. Cliente Node.js

### Instalação
```bash
npm install axios ws
```

### Código
```javascript
const axios = require('axios');
const WebSocket = require('ws');
const FormData = require('form-data');
const fs = require('fs');

class AllPressClient {
  constructor(baseUrl = 'http://localhost:8080', wsUrl = 'ws://localhost:8081') {
    this.baseUrl = baseUrl;
    this.wsUrl = wsUrl;
    this.ws = null;
  }

  // Conectar WebSocket para atualizações em tempo real
  connectWebSocket(onMessage) {
    this.ws = new WebSocket(this.wsUrl);
    
    this.ws.on('open', () => {
      console.log('WebSocket connected');
    });
    
    this.ws.on('message', (data) => {
      const message = JSON.parse(data);
      onMessage(message);
    });
    
    this.ws.on('error', (error) => {
      console.error('WebSocket error:', error);
    });
  }

  // Listar impressoras
  async getPrinters() {
    const response = await axios.get(`${this.baseUrl}/api/printers`);
    return response.data;
  }

  // Descobrir impressoras
  async discoverPrinters() {
    const response = await axios.post(`${this.baseUrl}/api/printers/discover`);
    return response.data;
  }

  // Enviar job de impressão
  async printFile(filePath, printerName, options = {}) {
    const form = new FormData();
    form.append('file', fs.createReadStream(filePath));
    form.append('printer', printerName);
    
    if (options.copies) form.append('copies', options.copies);
    if (options.color_mode) form.append('color_mode', options.color_mode);
    if (options.duplex) form.append('duplex', options.duplex);
    if (options.media_size) form.append('media_size', options.media_size);

    const response = await axios.post(`${this.baseUrl}/api/jobs`, form, {
      headers: form.getHeaders()
    });
    
    return response.data;
  }

  // Obter status de um job
  async getJobStatus(jobId) {
    const response = await axios.get(`${this.baseUrl}/api/jobs/${jobId}`);
    return response.data;
  }

  // Cancelar job
  async cancelJob(jobId) {
    const response = await axios.delete(`${this.baseUrl}/api/jobs/${jobId}`);
    return response.data;
  }

  // Listar jobs ativos
  async getActiveJobs() {
    const response = await axios.get(`${this.baseUrl}/api/jobs?status=active`);
    return response.data;
  }

  // Estatísticas
  async getStatistics() {
    const response = await axios.get(`${this.baseUrl}/api/stats`);
    return response.data;
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
    }
  }
}

// Exemplo de uso
async function main() {
  const client = new AllPressClient();

  // Conectar WebSocket para updates
  client.connectWebSocket((message) => {
    console.log('Event:', message);
    
    switch(message.type) {
      case 'job_progress':
        console.log(`Job ${message.job_id}: ${message.progress}%`);
        break;
      case 'job_completed':
        console.log(`Job ${message.job_id} completed!`);
        break;
      case 'printer_discovered':
        console.log(`New printer: ${message.printer.name}`);
        break;
    }
  });

  try {
    // Listar impressoras
    const printers = await client.getPrinters();
    console.log('Printers:', printers);

    // Imprimir documento
    const job = await client.printFile(
      './document.pdf',
      'HP_LaserJet',
      {
        copies: 2,
        color_mode: 'color',
        duplex: 'long-edge',
        media_size: 'A4'
      }
    );
    
    console.log('Print job created:', job.job_id);

    // Acompanhar status
    const checkStatus = setInterval(async () => {
      const status = await client.getJobStatus(job.job_id);
      console.log('Job status:', status.status, status.progress);
      
      if (status.status === 'Completed' || status.status === 'Failed') {
        clearInterval(checkStatus);
        client.disconnect();
      }
    }, 2000);

  } catch (error) {
    console.error('Error:', error.message);
    client.disconnect();
  }
}

// Executar
main();
```

## 2. Cliente Python

### Instalação
```bash
pip install requests websocket-client
```

### Código
```python
import requests
import json
import websocket
import threading
from typing import Optional, Dict, List

class AllPressClient:
    def __init__(self, base_url='http://localhost:8080', ws_url='ws://localhost:8081'):
        self.base_url = base_url
        self.ws_url = ws_url
        self.ws = None
        
    def connect_websocket(self, on_message):
        """Conectar ao WebSocket para atualizações em tempo real"""
        def on_ws_message(ws, message):
            data = json.loads(message)
            on_message(data)
        
        def on_error(ws, error):
            print(f"WebSocket error: {error}")
        
        def on_close(ws, close_status_code, close_msg):
            print("WebSocket closed")
        
        def on_open(ws):
            print("WebSocket connected")
        
        self.ws = websocket.WebSocketApp(
            self.ws_url,
            on_message=on_ws_message,
            on_error=on_error,
            on_close=on_close,
            on_open=on_open
        )
        
        ws_thread = threading.Thread(target=self.ws.run_forever)
        ws_thread.daemon = True
        ws_thread.start()
    
    def get_printers(self) -> List[Dict]:
        """Listar todas as impressoras"""
        response = requests.get(f"{self.base_url}/api/printers")
        response.raise_for_status()
        return response.json()
    
    def discover_printers(self) -> Dict:
        """Descobrir impressoras na rede"""
        response = requests.post(f"{self.base_url}/api/printers/discover")
        response.raise_for_status()
        return response.json()
    
    def print_file(self, file_path: str, printer_name: str, **options) -> Dict:
        """Enviar arquivo para impressão"""
        with open(file_path, 'rb') as f:
            files = {'file': f}
            data = {
                'printer': printer_name,
                **options
            }
            response = requests.post(
                f"{self.base_url}/api/jobs",
                files=files,
                data=data
            )
            response.raise_for_status()
            return response.json()
    
    def get_job_status(self, job_id: int) -> Dict:
        """Obter status de um job"""
        response = requests.get(f"{self.base_url}/api/jobs/{job_id}")
        response.raise_for_status()
        return response.json()
    
    def cancel_job(self, job_id: int) -> Dict:
        """Cancelar um job"""
        response = requests.delete(f"{self.base_url}/api/jobs/{job_id}")
        response.raise_for_status()
        return response.json()
    
    def get_active_jobs(self) -> List[Dict]:
        """Listar jobs ativos"""
        response = requests.get(f"{self.base_url}/api/jobs?status=active")
        response.raise_for_status()
        return response.json()
    
    def get_statistics(self) -> Dict:
        """Obter estatísticas do sistema"""
        response = requests.get(f"{self.base_url}/api/stats")
        response.raise_for_status()
        return response.json()
    
    def disconnect(self):
        """Desconectar WebSocket"""
        if self.ws:
            self.ws.close()

# Exemplo de uso
def main():
    client = AllPressClient()
    
    # Conectar WebSocket
    def handle_message(message):
        print(f"Event: {message}")
        
        if message['type'] == 'job_progress':
            print(f"Job {message['job_id']}: {message['progress']}%")
        elif message['type'] == 'job_completed':
            print(f"Job {message['job_id']} completed!")
    
    client.connect_websocket(handle_message)
    
    try:
        # Listar impressoras
        printers = client.get_printers()
        print(f"Printers: {printers}")
        
        # Imprimir documento
        job = client.print_file(
            './document.pdf',
            'HP_LaserJet',
            copies=2,
            color_mode='color',
            duplex='long-edge',
            media_size='A4'
        )
        
        print(f"Print job created: {job['job_id']}")
        
        # Acompanhar status
        import time
        while True:
            status = client.get_job_status(job['job_id'])
            print(f"Job status: {status['status']} - {status['progress']}%")
            
            if status['status'] in ['Completed', 'Failed']:
                break
            
            time.sleep(2)
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        client.disconnect()

if __name__ == '__main__':
    main()
```

## 3. Cliente React (Web)

```jsx
import React, { useState, useEffect } from 'react';
import axios from 'axios';

const API_URL = 'http://localhost:8080';
const WS_URL = 'ws://localhost:8081';

function PrintManager() {
  const [printers, setPrinters] = useState([]);
  const [jobs, setJobs] = useState([]);
  const [ws, setWs] = useState(null);

  useEffect(() => {
    // Conectar WebSocket
    const websocket = new WebSocket(WS_URL);
    
    websocket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      handleWebSocketMessage(data);
    };
    
    setWs(websocket);
    
    // Carregar impressoras
    loadPrinters();
    loadJobs();
    
    return () => {
      if (websocket) websocket.close();
    };
  }, []);

  const handleWebSocketMessage = (message) => {
    switch (message.type) {
      case 'job_progress':
        updateJobProgress(message.job_id, message.progress);
        break;
      case 'job_completed':
        loadJobs();
        break;
      case 'printer_discovered':
        loadPrinters();
        break;
    }
  };

  const loadPrinters = async () => {
    try {
      const response = await axios.get(`${API_URL}/api/printers`);
      setPrinters(response.data);
    } catch (error) {
      console.error('Error loading printers:', error);
    }
  };

  const loadJobs = async () => {
    try {
      const response = await axios.get(`${API_URL}/api/jobs?status=active`);
      setJobs(response.data);
    } catch (error) {
      console.error('Error loading jobs:', error);
    }
  };

  const handlePrint = async (file, printer) => {
    const formData = new FormData();
    formData.append('file', file);
    formData.append('printer', printer);
    formData.append('copies', 1);
    formData.append('color_mode', 'color');

    try {
      const response = await axios.post(`${API_URL}/api/jobs`, formData);
      console.log('Print job created:', response.data);
      loadJobs();
    } catch (error) {
      console.error('Error creating print job:', error);
    }
  };

  const cancelJob = async (jobId) => {
    try {
      await axios.delete(`${API_URL}/api/jobs/${jobId}`);
      loadJobs();
    } catch (error) {
      console.error('Error canceling job:', error);
    }
  };

  return (
    <div className="print-manager">
      <h1>All Press Print Manager</h1>
      
      <section className="printers">
        <h2>Printers</h2>
        {printers.map(printer => (
          <div key={printer.name} className="printer-card">
            <h3>{printer.name}</h3>
            <p>{printer.make_model}</p>
            <p>Status: {printer.is_online ? 'Online' : 'Offline'}</p>
          </div>
        ))}
      </section>

      <section className="jobs">
        <h2>Active Jobs</h2>
        {jobs.map(job => (
          <div key={job.job_id} className="job-card">
            <h3>Job #{job.job_id}</h3>
            <p>File: {job.original_filename}</p>
            <p>Printer: {job.printer_name}</p>
            <p>Progress: {job.progress}%</p>
            <button onClick={() => cancelJob(job.job_id)}>Cancel</button>
          </div>
        ))}
      </section>
    </div>
  );
}

export default PrintManager;
```

## 4. cURL - Linha de Comando

```bash
# Listar impressoras
curl http://localhost:8080/api/printers

# Descobrir impressoras
curl -X POST http://localhost:8080/api/printers/discover

# Imprimir documento
curl -X POST http://localhost:8080/api/jobs \
  -F "file=@document.pdf" \
  -F "printer=HP_LaserJet" \
  -F "copies=2" \
  -F "color_mode=color" \
  -F "duplex=long-edge" \
  -F "media_size=A4"

# Status de job
curl http://localhost:8080/api/jobs/123

# Cancelar job
curl -X DELETE http://localhost:8080/api/jobs/123

# Listar jobs ativos
curl http://localhost:8080/api/jobs?status=active

# Estatísticas
curl http://localhost:8080/api/stats
```

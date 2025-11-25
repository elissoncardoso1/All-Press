import { useState, useCallback, useEffect } from 'react';
import { useDropzone } from 'react-dropzone';
import { Upload as UploadIcon, File, X } from 'lucide-react';
import { useNavigate } from 'react-router-dom';
import { Card, CardContent } from '@/components/ui/Card';
import { Button } from '@/components/ui/Button';
import { Progress } from '@/components/ui/Progress';
import { Badge } from '@/components/ui/Badge';
import { formatBytes } from '@/lib/utils';
import { jobAPI } from '@/services/api';
import { usePrinterStore } from '@/store/printerStore';
import { useJobStore } from '@/store/jobStore';
import { useUploadStore } from '@/store/uploadStore';
import toast from 'react-hot-toast';
import type { UploadedFile, PrintOptions } from '@/types';

export const Upload = () => {
  const [files, setFiles] = useState<UploadedFile[]>([]);
  const [isProcessing, setIsProcessing] = useState(false);
  const [selectedPrinter, setSelectedPrinter] = useState<string>('');
  const [printOptions, setPrintOptions] = useState<PrintOptions>({
    copies: 1,
    paperSize: 'A4',
    colorMode: 'auto',
    duplex: 'none',
    orientation: 'portrait',
    quality: 'normal',
  });
  
  const navigate = useNavigate();
  const printers = usePrinterStore((state) => state.printers);
  const fetchPrinters = usePrinterStore((state) => state.fetchPrinters);
  const fetchJobs = useJobStore((state) => state.fetchJobs);
  const pendingFiles = useUploadStore((state) => state.pendingFiles);
  const clearPendingFiles = useUploadStore((state) => state.clearPendingFiles);

  // Carregar impressoras ao montar o componente
  useEffect(() => {
    if (printers.length === 0) {
      fetchPrinters();
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Selecionar primeira impressora online automaticamente
  useEffect(() => {
    if (printers.length > 0 && !selectedPrinter) {
      const onlinePrinter = printers.find((p) => p.status === 'online');
      if (onlinePrinter) {
        setSelectedPrinter(onlinePrinter.id);
      }
    }
  }, [printers, selectedPrinter]);

  // Carregar arquivos pendentes do store quando o componente montar
  useEffect(() => {
    if (pendingFiles.length > 0) {
      const newFiles: UploadedFile[] = pendingFiles.map((file) => ({
        id: Math.random().toString(36).substring(7),
        file,
        status: 'ready',
        progress: 0,
      }));
      setFiles((prev) => [...prev, ...newFiles]);
      clearPendingFiles();
    }
  }, [pendingFiles, clearPendingFiles]);

  const onDrop = useCallback((acceptedFiles: File[]) => {
    const newFiles: UploadedFile[] = acceptedFiles.map((file) => ({
      id: Math.random().toString(36).substring(7),
      file,
      status: 'ready',
      progress: 0,
    }));
    setFiles((prev) => [...prev, ...newFiles]);
  }, []);

  const { getRootProps, getInputProps, isDragActive } = useDropzone({
    onDrop,
    accept: {
      'application/pdf': ['.pdf'],
      'image/*': ['.png', '.jpg', '.jpeg'],
      'application/msword': ['.doc'],
      'application/vnd.openxmlformats-officedocument.wordprocessingml.document': ['.docx'],
    },
  });

  const removeFile = (id: string) => {
    setFiles((prev) => prev.filter((f) => f.id !== id));
  };

  const handleProcessFiles = async () => {
    console.log('🔄 Iniciando processamento de arquivos...');
    console.log('📁 Arquivos:', files);
    console.log('🖨️ Impressora selecionada:', selectedPrinter);
    console.log('⚙️ Opções:', printOptions);
    
    if (files.length === 0) {
      toast.error('Nenhum arquivo selecionado');
      return;
    }

    if (!selectedPrinter) {
      toast.error('Selecione uma impressora');
      return;
    }

    // Verificar se a impressora selecionada está disponível
    const printer = printers.find((p) => p.id === selectedPrinter);
    if (!printer) {
      toast.error('Impressora não encontrada');
      return;
    }

    if (printer.status !== 'online') {
      toast.error('A impressora selecionada não está online');
      return;
    }

    setIsProcessing(true);

    try {
      let successCount = 0;
      let errorCount = 0;

      // Processar cada arquivo
      for (const uploadedFile of files) {
        try {
          // Atualizar status para uploading
          setFiles((prev) =>
            prev.map((f) =>
              f.id === uploadedFile.id
                ? { ...f, status: 'uploading' as const, progress: 0 }
                : f
            )
          );

          // Criar job de impressão com as opções configuradas
          console.log(`📤 Enviando arquivo: ${uploadedFile.file.name}`);
          console.log(`   Tamanho: ${uploadedFile.file.size} bytes`);
          console.log(`   Tipo: ${uploadedFile.file.type}`);
          console.log(`   Impressora: ${selectedPrinter}`);
          console.log(`   Opções:`, printOptions);
          
          const job = await jobAPI.create(selectedPrinter, uploadedFile.file, printOptions);
          console.log(`✅ Job criado com sucesso:`, job);
          console.log(`   Job ID: ${job.id}`);
          console.log(`   Status: ${job.status}`);
          
          // Atualizar progresso para 100% e status para processing
          setFiles((prev) =>
            prev.map((f) =>
              f.id === uploadedFile.id
                ? { ...f, status: 'processing' as const, progress: 100 }
                : f
            )
          );
          
          successCount++;
          toast.success(`${uploadedFile.file.name} enviado com sucesso!`);
        } catch (error) {
          console.error(`❌ Erro ao processar ${uploadedFile.file.name}:`, error);
          console.error(`   Tipo do erro:`, typeof error);
          console.error(`   Erro completo:`, error);
          
          let errorMessage = 'Erro desconhecido ao processar arquivo';
          if (error instanceof Error) {
            errorMessage = error.message;
            console.error(`   Mensagem: ${error.message}`);
            console.error(`   Stack:`, error.stack);
          } else if (typeof error === 'object' && error !== null && 'response' in error) {
            const axiosError = error as { response?: { data?: { error?: string }; status?: number }; message?: string };
            console.error(`   Status HTTP: ${axiosError.response?.status}`);
            console.error(`   Dados da resposta:`, axiosError.response?.data);
            errorMessage = axiosError.response?.data?.error || axiosError.message || errorMessage;
          }
          console.error(`   Mensagem de erro final: ${errorMessage}`);
          setFiles((prev) =>
            prev.map((f) =>
              f.id === uploadedFile.id
                ? { ...f, status: 'error' as const }
                : f
            )
          );
          errorCount++;
          toast.error(`${uploadedFile.file.name}: ${errorMessage}`);
        }
      }

      // Atualizar lista de jobs
      await fetchJobs();

      // Mostrar resumo
      if (successCount > 0) {
        toast.success(`${successCount} arquivo(s) processado(s) com sucesso!`);
      }

      // Limpar arquivos após 2 segundos e redirecionar para jobs se houver sucesso
      if (successCount > 0) {
        setTimeout(() => {
          setFiles([]);
          navigate('/jobs');
        }, 2000);
      }
    } catch (error) {
      console.error('Erro ao processar arquivos:', error);
      toast.error('Erro ao processar arquivos');
    } finally {
      setIsProcessing(false);
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold tracking-tight">Upload de Arquivos</h1>
        <p className="text-muted-foreground">
          Faça upload de arquivos para impressão
        </p>
      </div>

      {/* Print Options */}
      <Card>
        <CardContent className="p-6">
          <h2 className="text-lg font-semibold mb-4">Configurações de Impressão</h2>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {/* Printer Selection */}
            <div className="flex flex-col gap-2">
              <label className="text-sm font-medium">Impressora</label>
              <div className="relative">
                <select
                  value={selectedPrinter}
                  onChange={(e) => setSelectedPrinter(e.target.value)}
                  className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 pr-10 text-sm ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:cursor-not-allowed disabled:opacity-50"
                >
                  <option value="">Selecione uma impressora</option>
                  {printers.map((printer) => (
                    <option key={printer.id} value={printer.id} disabled={printer.status !== 'online'}>
                      {printer.status === 'online' ? '🟢' : '🔴'} {printer.name}
                    </option>
                  ))}
                </select>
              </div>
              {/* Lista de impressoras com indicadores visuais */}
              <div className="mt-2 space-y-2 max-h-48 overflow-y-auto rounded-md border border-input p-2">
                {printers.length === 0 ? (
                  <p className="text-sm text-muted-foreground text-center py-2">
                    Nenhuma impressora disponível
                  </p>
                ) : (
                  printers.map((printer) => (
                    <button
                      key={printer.id}
                      type="button"
                      onClick={() => setSelectedPrinter(printer.id)}
                      disabled={printer.status !== 'online'}
                      className={`w-full flex items-center gap-2 px-3 py-2 rounded-md text-left transition-colors ${
                        selectedPrinter === printer.id
                          ? 'bg-primary text-primary-foreground'
                          : printer.status === 'online'
                          ? 'hover:bg-accent hover:text-accent-foreground'
                          : 'opacity-50 cursor-not-allowed'
                      }`}
                    >
                      <div className={`w-3 h-3 rounded-full ${
                        printer.status === 'online' ? 'bg-green-500' : 'bg-red-500'
                      }`} />
                      <div className="flex-1">
                        <div className="font-medium text-sm">{printer.name}</div>
                        <div className="text-xs opacity-75">
                          {printer.status === 'online' ? 'Disponível' : 'Indisponível'} • {printer.model || 'Sem modelo'}
                        </div>
                      </div>
                    </button>
                  ))
                )}
              </div>
            </div>

            {/* Paper Size */}
            <div className="flex flex-col gap-2">
              <label className="text-sm font-medium">Tamanho do Papel</label>
              <select
                value={printOptions.paperSize}
                onChange={(e) => setPrintOptions({ ...printOptions, paperSize: e.target.value })}
                className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2"
              >
                <option value="A4">A4</option>
                <option value="A3">A3</option>
                <option value="Letter">Letter</option>
                <option value="Legal">Legal</option>
                <option value="Tabloid">Tabloid</option>
              </select>
            </div>

            {/* Color Mode */}
            <div className="flex flex-col gap-2">
              <label className="text-sm font-medium">Cor</label>
              <select
                value={printOptions.colorMode}
                onChange={(e) => setPrintOptions({ ...printOptions, colorMode: e.target.value as PrintOptions['colorMode'] })}
                className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2"
              >
                <option value="auto">Automático</option>
                <option value="color">Colorido</option>
                <option value="monochrome">Preto e Branco</option>
              </select>
            </div>

            {/* Copies */}
            <div className="flex flex-col gap-2">
              <label className="text-sm font-medium">Cópias</label>
              <input
                type="number"
                min="1"
                max="99"
                value={printOptions.copies}
                onChange={(e) => setPrintOptions({ ...printOptions, copies: parseInt(e.target.value) || 1 })}
                className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2"
              />
            </div>

            {/* Duplex */}
            <div className="flex flex-col gap-2">
              <label className="text-sm font-medium">Frente e Verso</label>
              <select
                value={printOptions.duplex}
                onChange={(e) => setPrintOptions({ ...printOptions, duplex: e.target.value as PrintOptions['duplex'] })}
                className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2"
              >
                <option value="none">Simplex (Um Lado)</option>
                <option value="long-edge">Duplex (Borda Longa)</option>
                <option value="short-edge">Duplex (Borda Curta)</option>
              </select>
            </div>

            {/* Orientation */}
            <div className="flex flex-col gap-2">
              <label className="text-sm font-medium">Orientação</label>
              <select
                value={printOptions.orientation}
                onChange={(e) => setPrintOptions({ ...printOptions, orientation: e.target.value as PrintOptions['orientation'] })}
                className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2"
              >
                <option value="portrait">Retrato</option>
                <option value="landscape">Paisagem</option>
              </select>
            </div>

            {/* Quality */}
            <div className="flex flex-col gap-2">
              <label className="text-sm font-medium">Qualidade</label>
              <select
                value={printOptions.quality}
                onChange={(e) => setPrintOptions({ ...printOptions, quality: e.target.value as PrintOptions['quality'] })}
                className="flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2"
              >
                <option value="draft">Rascunho</option>
                <option value="normal">Normal</option>
                <option value="high">Alta</option>
              </select>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Drop Zone */}
      <Card>
        <CardContent className="p-6">
          <div
            {...getRootProps()}
            className={`cursor-pointer rounded-lg border-2 border-dashed p-12 text-center transition-colors ${
              isDragActive
                ? 'border-primary bg-primary/5'
                : 'border-muted-foreground/25 hover:border-primary/50'
            }`}
          >
            <input {...getInputProps()} />
            <UploadIcon className="mx-auto h-12 w-12 text-muted-foreground" />
            <h3 className="mt-4 text-lg font-semibold">
              {isDragActive ? 'Solte os arquivos aqui' : 'Arraste arquivos aqui'}
            </h3>
            <p className="mt-2 text-sm text-muted-foreground">
              ou clique para selecionar arquivos
            </p>
            <p className="mt-4 text-xs text-muted-foreground">
              Formatos suportados: PDF, DOC, DOCX, PNG, JPG
            </p>
          </div>
        </CardContent>
      </Card>

      {/* Files List */}
      {files.length > 0 && (
        <div className="space-y-4">
          <h2 className="text-xl font-semibold">Arquivos ({files.length})</h2>
          <div className="space-y-3">
            {files.map((uploadedFile) => (
              <Card key={uploadedFile.id}>
                <CardContent className="p-4">
                  <div className="flex items-center gap-4">
                    <div className="rounded-md bg-muted p-3">
                      <File className="h-6 w-6" />
                    </div>
                    <div className="flex-1 space-y-1">
                      <div className="flex items-center justify-between">
                        <h4 className="font-medium">{uploadedFile.file.name}</h4>
                        <Badge 
                          variant={
                            uploadedFile.status === 'error' 
                              ? 'error' 
                              : uploadedFile.status === 'processing' || uploadedFile.status === 'uploading'
                              ? 'info'
                              : uploadedFile.status === 'ready'
                              ? 'success'
                              : 'default'
                          }
                        >
                          {uploadedFile.status === 'ready' ? 'Pronto' :
                           uploadedFile.status === 'uploading' ? 'Enviando...' :
                           uploadedFile.status === 'processing' ? 'Processando...' :
                           uploadedFile.status === 'error' ? 'Erro' : uploadedFile.status}
                        </Badge>
                      </div>
                      <p className="text-sm text-muted-foreground">
                        {formatBytes(uploadedFile.file.size)}
                      </p>
                      {uploadedFile.status === 'uploading' && (
                        <Progress value={uploadedFile.progress} className="mt-2" />
                      )}
                    </div>
                    <Button
                      variant="ghost"
                      size="icon"
                      onClick={() => removeFile(uploadedFile.id)}
                    >
                      <X className="h-4 w-4" />
                    </Button>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>

          <div className="flex justify-end gap-2">
            <Button 
              variant="outline" 
              onClick={() => setFiles([])}
              disabled={isProcessing}
            >
              Limpar Todos
            </Button>
            <Button 
              onClick={handleProcessFiles}
              disabled={isProcessing || files.length === 0 || !selectedPrinter}
            >
              {isProcessing ? 'Processando...' : 'Processar Arquivos'}
            </Button>
          </div>
        </div>
      )}
    </div>
  );
};

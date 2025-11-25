import React, { useCallback } from 'react';
import { useDropzone } from 'react-dropzone';
import { useNavigate } from 'react-router-dom';
import { UploadCloud } from 'lucide-react';
import toast from 'react-hot-toast';
import { useUploadStore } from '@/store/uploadStore';

export const Dropzone: React.FC = () => {
    const navigate = useNavigate();
    const setPendingFiles = useUploadStore((state) => state.setPendingFiles);
    
    const onDrop = useCallback((acceptedFiles: File[]) => {
        if (acceptedFiles.length === 0) {
            toast.error('Nenhum arquivo válido foi selecionado');
            return;
        }
        
        // Armazenar arquivos no store
        setPendingFiles(acceptedFiles);
        
        // Redirecionar para a página de upload
        navigate('/upload');
        
        toast.success(`${acceptedFiles.length} arquivo(s) selecionado(s)`);
    }, [navigate, setPendingFiles]);

    const { getRootProps, getInputProps, isDragActive } = useDropzone({ 
        onDrop,
        accept: {
            'application/pdf': ['.pdf'],
            'image/*': ['.png', '.jpg', '.jpeg'],
            'application/msword': ['.doc'],
            'application/vnd.openxmlformats-officedocument.wordprocessingml.document': ['.docx'],
        },
    });

    return (
        <div
            {...getRootProps()}
            className={`
        relative group cursor-pointer overflow-hidden rounded-2xl border-2 border-dashed transition-all duration-300
        ${isDragActive
                    ? 'border-primary bg-primary/5 scale-[1.01]'
                    : 'border-border hover:border-primary/50 hover:bg-secondary/20'
                }
      `}
        >
            <input {...getInputProps()} />

            <div className="flex flex-col items-center justify-center py-12 px-4 text-center">
                <div className={`
          p-4 rounded-full mb-4 transition-all duration-300
          ${isDragActive ? 'bg-primary/20 text-primary' : 'bg-secondary text-muted-foreground group-hover:text-primary group-hover:bg-primary/10'}
        `}>
                    <UploadCloud size={32} />
                </div>

                <h3 className="text-lg font-semibold text-foreground mb-1">
                    {isDragActive ? 'Drop files here...' : 'Drag & Drop PDF files'}
                </h3>
                <p className="text-sm text-muted-foreground max-w-xs mx-auto">
                    Or click to browse. Optimized for high-volume processing.
                </p>
            </div>

            {/* Animated border effect */}
            <div className="absolute inset-0 pointer-events-none border-2 border-dashed border-primary/0 transition-all duration-300 group-hover:border-primary/20 rounded-2xl" />
        </div>
    );
};

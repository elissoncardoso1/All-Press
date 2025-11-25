import React, { useState, useRef, useEffect } from 'react';
import { Document, Page, pdfjs } from 'react-pdf';
import { ZoomIn, ZoomOut, RotateCw, CheckCircle } from 'lucide-react';
import 'react-pdf/dist/esm/Page/AnnotationLayer.css';
import 'react-pdf/dist/esm/Page/TextLayer.css';

// Configure PDF.js worker
pdfjs.GlobalWorkerOptions.workerSrc = `//cdnjs.cloudflare.com/ajax/libs/pdf.js/${pdfjs.version}/pdf.worker.min.js`;

interface PaperSize {
    name: string;
    width: number;  // in mm
    height: number; // in mm
    marginTop: number;
    marginRight: number;
    marginBottom: number;
    marginLeft: number;
}

const PAPER_SIZES: Record<string, PaperSize> = {
    A4: {
        name: 'A4',
        width: 210,
        height: 297,
        marginTop: 10,
        marginRight: 10,
        marginBottom: 10,
        marginLeft: 10,
    },
    Letter: {
        name: 'Letter',
        width: 215.9,
        height: 279.4,
        marginTop: 12.7,
        marginRight: 12.7,
        marginBottom: 12.7,
        marginLeft: 12.7,
    },
    Legal: {
        name: 'Legal',
        width: 215.9,
        height: 355.6,
        marginTop: 12.7,
        marginRight: 12.7,
        marginBottom: 12.7,
        marginLeft: 12.7,
    },
    A3: {
        name: 'A3',
        width: 297,
        height: 420,
        marginTop: 15,
        marginRight: 15,
        marginBottom: 15,
        marginLeft: 15,
    },
};

interface PrintPreviewProps {
    file: File | string;
    paperSize?: string;
}

export const PrintPreview: React.FC<PrintPreviewProps> = ({
    file,
    paperSize = 'A4',
}) => {
    const [numPages, setNumPages] = useState<number>(0);
    const [currentPage, setCurrentPage] = useState<number>(1);
    const [scale, setScale] = useState<number>(1.0);
    const [rotation, setRotation] = useState<number>(0);
    const [loading, setLoading] = useState<boolean>(true);
    const [error, setError] = useState<string | null>(null);
    const [showMargins, setShowMargins] = useState<boolean>(true);
    const containerRef = useRef<HTMLDivElement>(null);

    const paper = PAPER_SIZES[paperSize] || PAPER_SIZES.A4;

    // Calculate printable area
    const printableWidth = paper.width - paper.marginLeft - paper.marginRight;
    const printableHeight = paper.height - paper.marginTop - paper.marginBottom;

    const onDocumentLoadSuccess = ({ numPages }: { numPages: number }) => {
        setNumPages(numPages);
        setLoading(false);
        setError(null);
    };

    const onDocumentLoadError = (error: Error) => {
        console.error('Error loading PDF:', error);
        setError('Erro ao carregar o documento PDF');
        setLoading(false);
    };

    const handleZoomIn = () => {
        setScale(prev => Math.min(prev + 0.2, 3.0));
    };

    const handleZoomOut = () => {
        setScale(prev => Math.max(prev - 0.2, 0.5));
    };

    const handleRotate = () => {
        setRotation(prev => (prev + 90) % 360);
    };

    const handlePrevPage = () => {
        setCurrentPage(prev => Math.max(prev - 1, 1));
    };

    const handleNextPage = () => {
        setCurrentPage(prev => Math.min(prev + 1, numPages));
    };

    // Convert File to URL if needed
    const fileUrl = typeof file === 'string' ? file : URL.createObjectURL(file);

    useEffect(() => {
        return () => {
            if (typeof file !== 'string') {
                URL.revokeObjectURL(fileUrl);
            }
        };
    }, [file, fileUrl]);

    return (
        <div className="flex flex-col h-full bg-card border border-border rounded-lg overflow-hidden">
            {/* Toolbar */}
            <div className="flex items-center justify-between p-4 border-b border-border bg-muted/30">
                <div className="flex items-center gap-4">
                    <div className="flex items-center gap-2">
                        <button
                            onClick={handleZoomOut}
                            className="p-2 hover:bg-muted rounded-lg transition-colors"
                            title="Diminuir zoom"
                        >
                            <ZoomOut className="w-4 h-4" />
                        </button>
                        <span className="text-sm font-medium min-w-[60px] text-center">
                            {Math.round(scale * 100)}%
                        </span>
                        <button
                            onClick={handleZoomIn}
                            className="p-2 hover:bg-muted rounded-lg transition-colors"
                            title="Aumentar zoom"
                        >
                            <ZoomIn className="w-4 h-4" />
                        </button>
                    </div>

                    <div className="h-6 w-px bg-border" />

                    <button
                        onClick={handleRotate}
                        className="p-2 hover:bg-muted rounded-lg transition-colors"
                        title="Rotacionar"
                    >
                        <RotateCw className="w-4 h-4" />
                    </button>

                    <div className="h-6 w-px bg-border" />

                    <label className="flex items-center gap-2 cursor-pointer">
                        <input
                            type="checkbox"
                            checked={showMargins}
                            onChange={(e) => setShowMargins(e.target.checked)}
                            className="rounded border-border"
                        />
                        <span className="text-sm">Mostrar margens</span>
                    </label>
                </div>

                <div className="flex items-center gap-4">
                    <div className="flex items-center gap-2">
                        <span className="text-sm text-muted-foreground">Papel:</span>
                        <span className="text-sm font-medium">{paper.name}</span>
                    </div>

                    {numPages > 0 && (
                        <>
                            <div className="h-6 w-px bg-border" />
                            <div className="flex items-center gap-2">
                                <button
                                    onClick={handlePrevPage}
                                    disabled={currentPage === 1}
                                    className="px-3 py-1 text-sm hover:bg-muted rounded transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                                >
                                    Anterior
                                </button>
                                <span className="text-sm">
                                    {currentPage} / {numPages}
                                </span>
                                <button
                                    onClick={handleNextPage}
                                    disabled={currentPage === numPages}
                                    className="px-3 py-1 text-sm hover:bg-muted rounded transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                                >
                                    Próxima
                                </button>
                            </div>
                        </>
                    )}
                </div>
            </div>

            {/* Margin Info */}
            {showMargins && (
                <div className="px-4 py-2 bg-blue-50 border-b border-blue-200 flex items-center gap-2 text-sm">
                    <CheckCircle className="w-4 h-4 text-blue-600" />
                    <span className="text-blue-800">
                        Área de impressão: {printableWidth.toFixed(0)}mm × {printableHeight.toFixed(0)}mm
                        (margens: {paper.marginTop}mm)
                    </span>
                </div>
            )}

            {/* Preview Area */}
            <div
                ref={containerRef}
                className="flex-1 overflow-auto bg-gray-100 p-8 flex items-center justify-center"
            >
                {loading && (
                    <div className="text-center">
                        <div className="inline-block animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
                        <p className="mt-2 text-sm text-muted-foreground">Carregando documento...</p>
                    </div>
                )}

                {error && (
                    <div className="bg-red-50 border border-red-200 rounded-lg p-4 text-red-600 max-w-md">
                        <p className="font-medium">Erro ao carregar preview</p>
                        <p className="text-sm mt-1">{error}</p>
                    </div>
                )}

                {!loading && !error && (
                    <div className="relative">
                        {/* Paper background with shadow */}
                        <div className="bg-white shadow-2xl" style={{ transform: `rotate(${rotation}deg)` }}>
                            <Document
                                file={fileUrl}
                                onLoadSuccess={onDocumentLoadSuccess}
                                onLoadError={onDocumentLoadError}
                                loading={null}
                                error={null}
                            >
                                <Page
                                    pageNumber={currentPage}
                                    scale={scale}
                                    renderTextLayer={true}
                                    renderAnnotationLayer={true}
                                />
                            </Document>

                            {/* Margin overlay */}
                            {showMargins && (
                                <div
                                    className="absolute inset-0 pointer-events-none"
                                    style={{
                                        border: '2px dashed rgba(59, 130, 246, 0.5)',
                                        margin: `${paper.marginTop * scale * 3.78}px ${paper.marginRight * scale * 3.78}px ${paper.marginBottom * scale * 3.78}px ${paper.marginLeft * scale * 3.78}px`,
                                    }}
                                >
                                    <div className="absolute -top-6 left-0 text-xs text-blue-600 bg-blue-50 px-2 py-1 rounded">
                                        Área de impressão
                                    </div>
                                </div>
                            )}
                        </div>
                    </div>
                )}
            </div>
        </div>
    );
};

#include "core/job_queue.h"
#include "protocols/protocol_factory.h"
#include "protocols/compatibility_matrix.h"
#include "utils/logger.h"
#include "utils/file_utils.h"
#include <fstream>
#include <sstream>

namespace AllPress {

using namespace all_press::protocols;

// Validar compatibilidade do job com o plotter
bool JobQueue::validate_job_compatibility(const PrintJob& job) {
    if (!printer_manager_) {
        LOG_ERROR("PrinterManager not set");
        return false;
    }
    
    // Verificar se é um plotter
    if (!printer_manager_->is_plotter(job.printer_name)) {
        // Não é plotter, usar validação padrão
        return true;
    }
    
    std::ostringstream oss;
    oss << "Validating job " << job.job_id << " for plotter " << job.printer_name;
    LOG_INFO(oss.str());
    
    // Validar documento contra capacidades do plotter
    bool valid = printer_manager_->validate_document_for_plotter(
        job.printer_name,
        job.file_path,
        job.options);
    
    if (!valid) {
        std::ostringstream oss2;
        oss2 << "Job " << job.job_id << " failed compatibility check for plotter " << job.printer_name;
        LOG_ERROR(oss2.str());
    }
    
    return valid;
}

// Processar job com conversão de protocolo
void JobQueue::process_job_with_protocol(const ProcessingContext& context) {
    std::ostringstream oss;
    oss << "Processing job " << context.job_id << " with protocol " << context.target_protocol;
    LOG_INFO(oss.str());
    
    // Atualizar status
    update_job_status(context.job_id, JobStatus::Processing);
    
    try {
        // Ler arquivo original
        std::ifstream file(context.job.file_path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + context.job.file_path);
        }
        
        std::vector<uint8_t> file_data(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();
        
        // Converter media_size para enum
        MediaSize media_size = MediaSize::A4;
        if (context.job.options.media_size == "A0") media_size = MediaSize::A0;
        else if (context.job.options.media_size == "A1") media_size = MediaSize::A1;
        else if (context.job.options.media_size == "A2") media_size = MediaSize::A2;
        else if (context.job.options.media_size == "A3") media_size = MediaSize::A3;
        
        // Converter color_mode para enum
        ColorMode color_mode = ColorMode::MONOCHROME;
        if (context.job.options.color_mode == "color") {
            color_mode = ColorMode::COLOR;
        }
        
        // Calcular DPI baseado na qualidade
        int dpi = 300;
        if (context.job.options.quality == 5) dpi = 1200;
        else if (context.job.options.quality == 4) dpi = 600;
        else if (context.job.options.quality >= 3) dpi = 600;
        
        // Gerar header
        auto header = context.protocol_handler->generate_header(
            context.target_capabilities,
            media_size,
            color_mode,
            dpi);
        
        // Processar página (assumindo raster data)
        // Nota: Aqui você precisaria converter o arquivo para raster se necessário
        int width = 2480;  // A4 @ 300 DPI width
        int height = 3508; // A4 @ 300 DPI height
        
        auto page_data = context.protocol_handler->generate_page(
            file_data, width, height, dpi);
        
        // Gerar footer
        auto footer = context.protocol_handler->generate_footer();
        
        // Combinar tudo
        std::vector<uint8_t> final_data;
        final_data.insert(final_data.end(), header.begin(), header.end());
        final_data.insert(final_data.end(), page_data.begin(), page_data.end());
        final_data.insert(final_data.end(), footer.begin(), footer.end());
        
        // Otimizar para vendor
        final_data = context.protocol_handler->optimize_for_vendor(final_data);
        
        // Salvar arquivo temporário convertido
        std::string temp_file = context.job.file_path + ".converted";
        std::ofstream out_file(temp_file, std::ios::binary);
        out_file.write(reinterpret_cast<const char*>(final_data.data()), 
                      final_data.size());
        out_file.close();
        
        std::ostringstream oss;
        oss << "Job " << context.job_id << " converted to " << context.target_protocol 
            << " protocol, saved to " << temp_file;
        LOG_INFO(oss.str());
        
        // Aqui você enviaria o arquivo convertido para impressão
        // Por exemplo, via CUPS ou IPP
        
        update_job_status(context.job_id, JobStatus::Printing);
        
        // Simular progresso de impressão
        for (int i = 0; i <= 100; i += 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (progress_callback_) {
                progress_callback_(context.job_id, i / 100.0f);
            }
        }
        
        // Limpar arquivo temporário
        std::remove(temp_file.c_str());
        
        update_job_status(context.job_id, JobStatus::Completed);
        std::ostringstream oss2;
        oss2 << "Job " << context.job_id << " completed successfully";
        LOG_INFO(oss2.str());
        
    } catch (const std::exception& e) {
        std::ostringstream oss3;
        oss3 << "Failed to process job " << context.job_id << ": " << e.what();
        LOG_ERROR(oss3.str());
        update_job_status(context.job_id, JobStatus::Failed, e.what());
    }
}

} // namespace AllPress


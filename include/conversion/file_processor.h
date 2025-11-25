#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <mutex>

namespace AllPress {

enum class FileType {
    Unknown,
    PDF,
    Image,
    Office,
    CAD,
    Archive,
    SVG,
    Design,    // AI, PSD, CDR
    Text
};

struct FileInfo {
    std::string file_path;
    std::string mime_type;
    FileType type;
    size_t size_bytes;
    int estimated_pages;
    std::string dimensions; // "width x height"
    int dpi;
    bool has_color;
};

struct ConversionOptions {
    int target_dpi = 300;
    bool compress = true;
    std::string color_profile;
    bool preserve_transparency = false;
    std::string output_format = "PDF";
    int max_width = 0;  // 0 = no limit
    int max_height = 0;
};

class FileProcessor {
public:
    FileProcessor();
    ~FileProcessor();
    
    // Análise de arquivo
    FileInfo analyze_file(const std::string& file_path);
    FileType detect_file_type(const std::string& file_path);
    std::string detect_mime_type(const std::string& file_path);
    
    // Conversão
    std::future<std::string> convert_to_pdf_async(const std::string& input_path,
                                                 const ConversionOptions& options = {});
    std::string convert_to_pdf(const std::string& input_path, 
                              const ConversionOptions& options = {});
    
    // Processadores específicos
    std::string convert_image_to_pdf(const std::string& input_path,
                                    const ConversionOptions& options);

    std::string convert_office_to_pdf(const std::string& input_path,
                                     const ConversionOptions& options);

    std::string convert_cad_to_pdf(const std::string& input_path,
                                  const ConversionOptions& options);

    std::string convert_design_to_pdf(const std::string& input_path,
                                     const ConversionOptions& options);

    // Otimização
    std::string optimize_pdf_for_printing(const std::string& pdf_path,
                                          const ConversionOptions& options);
    
    // Preview
    std::string generate_preview_image(const std::string& file_path,
                                      int width = 200, int height = 200);

private:
    std::string temp_dir_;
    std::mutex conversion_mutex_;
};

} // namespace AllPress

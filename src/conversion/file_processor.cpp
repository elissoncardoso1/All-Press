#include "conversion/file_processor.h"
#include "utils/logger.h"
#include "utils/file_utils.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>

namespace AllPress {

FileProcessor::FileProcessor() {
    temp_dir_ = Utils::FileUtils::get_temp_directory() + "/all_press";
    Utils::FileUtils::create_directories(temp_dir_);
}

FileProcessor::~FileProcessor() {
}

FileInfo FileProcessor::analyze_file(const std::string& file_path) {
    FileInfo info;
    info.file_path = file_path;
    info.size_bytes = Utils::FileUtils::get_file_size(file_path);
    info.type = detect_file_type(file_path);
    info.mime_type = detect_mime_type(file_path);
    info.dpi = 300;
    info.has_color = true;
    info.estimated_pages = 1;
    
    LOG_INFO("Analyzed file: " + file_path + " (" + std::to_string(info.size_bytes) + " bytes)");
    return info;
}

FileType FileProcessor::detect_file_type(const std::string& file_path) {
    std::string ext = Utils::FileUtils::get_file_extension(file_path);

    if (ext == ".pdf") return FileType::PDF;
    if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" ||
        ext == ".bmp" || ext == ".tiff" || ext == ".tif") return FileType::Image;
    if (ext == ".doc" || ext == ".docx" || ext == ".odt" ||
        ext == ".xls" || ext == ".xlsx" || ext == ".ppt" || ext == ".pptx") return FileType::Office;
    if (ext == ".dwg" || ext == ".dxf" || ext == ".dwf" || ext == ".dgn" ||
        ext == ".plt" || ext == ".hpgl") return FileType::CAD;
    if (ext == ".svg") return FileType::SVG;
    if (ext == ".ai" || ext == ".psd" || ext == ".cdr" || ext == ".eps" ||
        ext == ".afdesign" || ext == ".sketch") return FileType::Design;
    if (ext == ".zip" || ext == ".rar" || ext == ".tar" || ext == ".gz") return FileType::Archive;
    if (ext == ".txt" || ext == ".rtf" || ext == ".md") return FileType::Text;

    return FileType::Unknown;
}

std::string FileProcessor::detect_mime_type(const std::string& file_path) {
    FileType type = detect_file_type(file_path);

    switch (type) {
        case FileType::PDF:     return "application/pdf";
        case FileType::Image:   return "image/jpeg";
        case FileType::Office:  return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        case FileType::CAD:     return "application/x-dwg";
        case FileType::SVG:     return "image/svg+xml";
        case FileType::Design:  return "application/x-design";
        case FileType::Archive: return "application/zip";
        case FileType::Text:    return "text/plain";
        default:               return "application/octet-stream";
    }
}

std::future<std::string> FileProcessor::convert_to_pdf_async(const std::string& input_path,
                                                            const ConversionOptions& options) {
    return std::async(std::launch::async, [this, input_path, options]() {
        return convert_to_pdf(input_path, options);
    });
}

std::string FileProcessor::convert_to_pdf(const std::string& input_path, 
                                         const ConversionOptions& options) {
    FileType type = detect_file_type(input_path);
    
    LOG_INFO("Converting file to PDF: " + input_path);
    
    std::string output_path;
    
    switch (type) {
        case FileType::PDF:
            // Already PDF, optionally optimize
            output_path = input_path;
            if (options.compress) {
                output_path = optimize_pdf_for_printing(input_path, options);
            }
            break;
            
        case FileType::Image:
            output_path = convert_image_to_pdf(input_path, options);
            break;

        case FileType::Office:
            output_path = convert_office_to_pdf(input_path, options);
            break;

        case FileType::CAD:
            output_path = convert_cad_to_pdf(input_path, options);
            break;

        case FileType::Design:
            output_path = convert_design_to_pdf(input_path, options);
            break;

        case FileType::Text:
            // Convert text files to PDF using pandoc
            output_path = convert_office_to_pdf(input_path, options);
            break;

        default:
            LOG_WARNING("Unsupported file type for conversion");
            output_path = input_path;
            break;
    }
    
    return output_path;
}

std::string FileProcessor::convert_image_to_pdf(const std::string& input_path, 
                                               const ConversionOptions& options) {
    std::lock_guard<std::mutex> lock(conversion_mutex_);
    
    std::string output_path = temp_dir_ + "/" + 
                             Utils::FileUtils::get_filename(input_path) + ".pdf";
    
    LOG_INFO("Converting image to PDF: " + input_path + " -> " + output_path);
    
    // Implementation would use ImageMagick or similar:
    // system(("convert " + input_path + " " + output_path).c_str());
    
    // For now, just copy the file (stub implementation)
    Utils::FileUtils::copy_file(input_path, output_path);
    
    return output_path;
}

std::string FileProcessor::convert_office_to_pdf(const std::string& input_path,
                                                const ConversionOptions& options) {
    std::lock_guard<std::mutex> lock(conversion_mutex_);

    std::string filename = Utils::FileUtils::get_filename(input_path);
    std::string base_name = filename.substr(0, filename.find_last_of('.'));
    std::string output_path = temp_dir_ + "/" + base_name + ".pdf";

    LOG_INFO("Converting Office document to PDF: " + input_path + " -> " + output_path);

    // Try Pandoc first (preferred method for .docx files)
    std::string pandoc_cmd = "pandoc \"" + input_path + "\" -o \"" + output_path + "\" 2>&1";

    int result = system(pandoc_cmd.c_str());

    if (result == 0 && std::filesystem::exists(output_path)) {
        LOG_INFO("Successfully converted document using Pandoc");

        // Apply optimization if requested
        if (options.compress) {
            output_path = optimize_pdf_for_printing(output_path, options);
        }

        return output_path;
    }

    // Fallback: Try LibreOffice (if available)
    std::string libreoffice_cmd = "libreoffice --headless --convert-to pdf --outdir \"" +
                                 temp_dir_ + "\" \"" + input_path + "\" 2>&1";

    result = system(libreoffice_cmd.c_str());

    // LibreOffice creates files with the same base name but .pdf extension
    std::string lo_output_path = temp_dir_ + "/" + base_name + ".pdf";

    if (result == 0 && std::filesystem::exists(lo_output_path)) {
        LOG_INFO("Successfully converted document using LibreOffice");

        // Apply optimization if requested
        if (options.compress) {
            lo_output_path = optimize_pdf_for_printing(lo_output_path, options);
        }

        return lo_output_path;
    }

    LOG_ERROR("Failed to convert Office document to PDF: " + input_path);
    return input_path; // Return original file if conversion fails
}

std::string FileProcessor::convert_cad_to_pdf(const std::string& input_path,
                                               const ConversionOptions& options) {
    std::lock_guard<std::mutex> lock(conversion_mutex_);

    std::string filename = Utils::FileUtils::get_filename(input_path);
    std::string base_name = filename.substr(0, filename.find_last_of('.'));
    std::string output_path = temp_dir_ + "/" + base_name + ".pdf";
    std::string ext = Utils::FileUtils::get_file_extension(input_path);

    LOG_INFO("Converting CAD file to PDF: " + input_path + " -> " + output_path);

    // Different approaches based on CAD file type
    if (ext == ".dwg" || ext == ".dxf") {
        // Try using ODA File Converter if available (commercial)
        std::string oda_cmd = "ODAFileConverter \"" + temp_dir_ + "\" \"" +
                              Utils::FileUtils::get_directory(input_path) +
                              "\" \"ACAD2018\" \"PDF\" \"0\" \"0\"";
        int result = system(oda_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted CAD file using ODA File Converter");
            return output_path;
        }

        // Fallback: Try using LibreOffice with Draw
        std::string lo_cmd = "libreoffice --headless --convert-to pdf --outdir \"" +
                            temp_dir_ + "\" \"" + input_path + "\" 2>&1";
        result = system(lo_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted CAD file using LibreOffice Draw");
            return output_path;
        }
    }
    else if (ext == ".svg") {
        // SVG to PDF using ImageMagick
        std::string convert_cmd = "magick convert \"" + input_path + "\" \"" + output_path + "\" 2>&1";
        int result = system(convert_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted SVG to PDF using ImageMagick");
            return output_path;
        }
    }
    else if (ext == ".hpgl" || ext == ".plt") {
        // HPGL/PLT files - try using Ghostscript
        std::string gs_cmd = "gs -sDEVICE=pdfwrite -sOutputFile=\"" + output_path +
                            "\" -dBATCH -dNOPAUSE \"" + input_path + "\" 2>&1";
        int result = system(gs_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted HPGL/PLT to PDF using Ghostscript");
            return output_path;
        }
    }

    LOG_ERROR("Failed to convert CAD file to PDF: " + input_path);
    return input_path; // Return original file if conversion fails
}

std::string FileProcessor::convert_design_to_pdf(const std::string& input_path,
                                                 const ConversionOptions& options) {
    std::lock_guard<std::mutex> lock(conversion_mutex_);

    std::string filename = Utils::FileUtils::get_filename(input_path);
    std::string base_name = filename.substr(0, filename.find_last_of('.'));
    std::string output_path = temp_dir_ + "/" + base_name + ".pdf";
    std::string ext = Utils::FileUtils::get_file_extension(input_path);

    LOG_INFO("Converting Design file to PDF: " + input_path + " -> " + output_path);

    // Try different approaches based on file type
    if (ext == ".psd") {
        // PSD to PDF using ImageMagick
        std::string convert_cmd = "magick convert \"" + input_path + "\" \"" + output_path + "\" 2>&1";
        int result = system(convert_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted PSD to PDF using ImageMagick");
            return output_path;
        }
    }
    else if (ext == ".ai") {
        // AI files - try multiple methods
        // 1. Try ImageMagick first (if AI is saved with PDF compatibility)
        std::string convert_cmd = "magick convert \"" + input_path + "\" \"" + output_path + "\" 2>&1";
        int result = system(convert_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted AI to PDF using ImageMagick");
            return output_path;
        }

        // 2. Try using Inkscape if available (not installed in this case)
        std::string inkscape_cmd = "inkscape --export-pdf=\"" + output_path + "\" \"" + input_path + "\" 2>&1";
        result = system(inkscape_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted AI to PDF using Inkscape");
            return output_path;
        }
    }
    else if (ext == ".eps") {
        // EPS to PDF using Ghostscript
        std::string gs_cmd = "gs -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=\"" +
                            output_path + "\" \"" + input_path + "\" 2>&1";
        int result = system(gs_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted EPS to PDF using Ghostscript");
            return output_path;
        }

        // Fallback: Try ImageMagick
        std::string convert_cmd = "magick convert \"" + input_path + "\" \"" + output_path + "\" 2>&1";
        result = system(convert_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted EPS to PDF using ImageMagick");
            return output_path;
        }
    }
    else if (ext == ".cdr") {
        // CorelDRAW files - try using LibreOffice
        std::string lo_cmd = "libreoffice --headless --convert-to pdf --outdir \"" +
                            temp_dir_ + "\" \"" + input_path + "\" 2>&1";
        int result = system(lo_cmd.c_str());

        if (result == 0 && std::filesystem::exists(output_path)) {
            LOG_INFO("Successfully converted CDR to PDF using LibreOffice");
            return output_path;
        }
    }

    LOG_ERROR("Failed to convert Design file to PDF: " + input_path);
    return input_path; // Return original file if conversion fails
}

std::string FileProcessor::optimize_pdf_for_printing(const std::string& pdf_path,
                                                    const ConversionOptions& options) {
    std::string output_path = temp_dir_ + "/optimized_" + 
                             Utils::FileUtils::get_filename(pdf_path);
    
    LOG_INFO("Optimizing PDF for printing: " + pdf_path);
    
    // Implementation would use Ghostscript:
    // gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.4 -dPDFSETTINGS=/printer ...
    
    Utils::FileUtils::copy_file(pdf_path, output_path);
    return output_path;
}

std::string FileProcessor::generate_preview_image(const std::string& file_path,
                                                 int width, int height) {
    std::string output_path = temp_dir_ + "/preview_" + 
                             Utils::FileUtils::get_filename(file_path) + ".jpg";
    
    LOG_INFO("Generating preview: " + file_path);
    
    // Implementation would use ImageMagick or Poppler
    
    return output_path;
}

} // namespace AllPress

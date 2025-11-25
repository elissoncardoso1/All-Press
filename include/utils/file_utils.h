#pragma once

#include <string>
#include <vector>

namespace AllPress::Utils {

class FileUtils {
public:
    static bool file_exists(const std::string& path);
    static bool directory_exists(const std::string& path);
    static bool create_directory(const std::string& path);
    static bool create_directories(const std::string& path);
    static bool remove_file(const std::string& path);
    static bool remove_directory(const std::string& path);
    
    static size_t get_file_size(const std::string& path);
    static std::string get_file_extension(const std::string& path);
    static std::string get_filename(const std::string& path);
    static std::string get_directory(const std::string& path);
    
    static std::string read_file(const std::string& path);
    static bool write_file(const std::string& path, const std::string& content);
    
    static std::vector<std::string> list_files(const std::string& directory, 
                                               const std::string& pattern = "*");
    
    static std::string get_temp_directory();
    static std::string create_temp_file(const std::string& prefix = "allpress_", 
                                       const std::string& extension = ".tmp");
    
    static bool copy_file(const std::string& source, const std::string& destination);
    static bool move_file(const std::string& source, const std::string& destination);
};

} // namespace AllPress::Utils

#include "utils/file_utils.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

namespace AllPress {
namespace Utils {

bool FileUtils::file_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

bool FileUtils::directory_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

bool FileUtils::create_directory(const std::string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}

bool FileUtils::create_directories(const std::string& path) {
    if (directory_exists(path)) {
        return true;
    }
    
    size_t pos = 0;
    std::string current_path;
    
    while ((pos = path.find('/', pos)) != std::string::npos) {
        current_path = path.substr(0, pos);
        if (!current_path.empty() && !directory_exists(current_path)) {
            if (!create_directory(current_path)) {
                return false;
            }
        }
        pos++;
    }
    
    return create_directory(path);
}

bool FileUtils::remove_file(const std::string& path) {
    return unlink(path.c_str()) == 0;
}

bool FileUtils::remove_directory(const std::string& path) {
    return rmdir(path.c_str()) == 0;
}

size_t FileUtils::get_file_size(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return buffer.st_size;
    }
    return 0;
}

std::string FileUtils::get_file_extension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        return path.substr(pos);
    }
    return "";
}

std::string FileUtils::get_filename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

std::string FileUtils::get_directory(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(0, pos);
    }
    return ".";
}

std::string FileUtils::read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool FileUtils::write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return true;
}

std::vector<std::string> FileUtils::list_files(const std::string& directory, 
                                               const std::string& pattern) {
    std::vector<std::string> files;
    
    DIR* dir = opendir(directory.c_str());
    if (!dir) {
        return files;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            files.push_back(entry->d_name);
        }
    }
    
    closedir(dir);
    return files;
}

std::string FileUtils::get_temp_directory() {
#ifdef _WIN32
    const char* temp = getenv("TEMP");
    if (temp) return std::string(temp);
    return "C:\\Temp";
#else
    const char* temp = getenv("TMPDIR");
    if (temp) return std::string(temp);
    return "/tmp";
#endif
}

std::string FileUtils::create_temp_file(const std::string& prefix, 
                                       const std::string& extension) {
    std::string temp_dir = get_temp_directory();
    std::string temp_file = temp_dir + "/" + prefix + 
                           std::to_string(std::time(nullptr)) + extension;
    return temp_file;
}

bool FileUtils::copy_file(const std::string& source, const std::string& destination) {
    std::ifstream src(source, std::ios::binary);
    if (!src.is_open()) {
        return false;
    }
    
    std::ofstream dst(destination, std::ios::binary);
    if (!dst.is_open()) {
        return false;
    }
    
    dst << src.rdbuf();
    return true;
}

bool FileUtils::move_file(const std::string& source, const std::string& destination) {
    if (rename(source.c_str(), destination.c_str()) == 0) {
        return true;
    }
    
    // If rename fails, try copy and delete
    if (copy_file(source, destination)) {
        return remove_file(source);
    }
    
    return false;
}

} // namespace Utils
} // namespace AllPress

#include "utils.h"

#include <sstream>
#include <filesystem>

std::vector<std::string> SplitString(const std::string& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.push_back(item);
        }
    }

    return elems;
}

std::string ExtractBasePath(const std::string& path) {
    std::filesystem::path filepath(path);
    return filepath.parent_path();
}

std::string ExtractFilenameWithoutExt(const std::string& path) {
    std::filesystem::path filepath(path);
    std::string filename = filepath.filename().string();
    size_t dot_pos = filename.find_last_of('.');

    if (dot_pos == std::string::npos) {
        return filename;
    }
    
    return filename.substr(0, dot_pos);
}

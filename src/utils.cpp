#include "utils.h"

#include <sstream>
#include <string.h>

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
    const char* cpath = path.c_str();
    const char* last_slash = strrchr(cpath, PATH_DELIMITER);

    std::string base_path;
    if (last_slash != nullptr) {
        size_t parent_path_len = last_slash - cpath;
        base_path = path.substr(0, parent_path_len);
    }

    return base_path;
}

std::string ExtractFilenameWithoutExt(const std::string& path) {
    size_t len = path.length();
    const char* cpath = path.c_str();
    const char* start = strrchr(cpath, PATH_DELIMITER);
    const char* end = strrchr(cpath, '.');
    
    if (end == nullptr) {
        end = &cpath[len];
    }
    if (start == nullptr) {
        start = &cpath[0];
    } else {
        start = start + 1;
    }

    std::string filename;
    size_t filename_len = end - start;
    size_t filename_start = start - cpath;
    filename = path.substr(filename_start, filename_len);

    return filename;
}

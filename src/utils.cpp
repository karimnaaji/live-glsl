#include "utils.h"

#include <sstream>

std::vector<std::string> SplitString(const std::string& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty() && item != ";") {
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
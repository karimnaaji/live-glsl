#include "utils.h"

char** strSplit(char* str, const char delimiter) {
    char** result = 0;
    size_t count = 0;
    char* tmp = str;
    char* last = 0;
    char delim[2];
    delim[0] = delimiter;
    delim[1] = 0;

    while(*tmp) {
        if (delimiter == *tmp) {
            count++;
            last = tmp;
        }
        tmp++;
    }

    count += last < (str + strlen(str) - 1);

    count++;
    result = (char**) malloc(sizeof(char*) * count);

    if(result) {
        size_t idx  = 0;
        char* token = strtok(str, delim);

        while(token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(NULL, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

std::vector<std::string> strSplit(std::string str, char delimiter) {
    std::vector<std::string> split;
    char* cstr = (char*) malloc(str.size() - 1);
    strcpy(cstr, str.c_str());
    char** csplit = strSplit(cstr, delimiter);

    if(csplit) {
        int i;
        for(i = 0; *(csplit + i); i++) {
            char* next = *(csplit + i);
            split.push_back(std::string(next));

            free(next);
        }
        free(csplit);
    }

    delete[] cstr;
    return split;
}

bool loadFromPath(const std::string& path, std::string* into) {
    std::ifstream file;
    std::string buffer;

    file.open(path.c_str());
    if(!file.is_open()) return false;
    while(!file.eof()) {
        getline(file, buffer);
        (*into) += buffer + "\n";
    }

    file.close();
    return true;
}


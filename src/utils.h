#pragma once

#include <string>
#include <vector>

template <typename T, unsigned long N>
char(&ArrayLength(T (&array)[N]))[N];

#define ARRAY_LENGTH(array) (sizeof(ArrayLength(array)))

#ifdef _WIN32
#define PATH_DELIMITER '\\'
#else
#define PATH_DELIMITER '/'
#endif

std::vector<std::string> SplitString(const std::string& s, char delim);
std::string ExtractBasePath(const std::string& path);
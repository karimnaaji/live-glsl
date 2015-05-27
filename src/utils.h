#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

std::vector<std::string> split(const std::string& s, char delim, std::vector<std::string>& elems);
std::vector<std::string> strSplit(const std::string& s, char delim);
bool loadFromPath(const std::string& path, std::string* into);

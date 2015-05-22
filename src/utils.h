#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>

char** strSplit(char* str, const char delimiter);

std::vector<std::string> strSplit(std::string str, char delimiter);

bool loadFromPath(const std::string& path, std::string* into);


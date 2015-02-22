#pragma once

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <cassert>

void redefineSignal(int sig, void (*handler)(int));

key_t createKey(const char* name);

char** strSplit(char* str, const char delimiter);

std::vector<std::string> strSplit(std::string str, char delimiter);

bool loadFromPath(const std::string& path, std::string* into);


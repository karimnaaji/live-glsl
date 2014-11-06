#pragma once

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdio.h>
#include <vector>

static void redefineSignal(int sig, void (*handler)(int)) {
    struct sigaction action;
    sigset_t set;

    sigemptyset(&set);
    action.sa_handler = handler;
    action.sa_flags = 0;
    action.sa_mask = set;

    if (sigaction(sig, &action, NULL) != 0) {
        std::cerr << "Not able to redefine signal" << std::endl;
    }
}

static key_t createKey(const char* name) {
    key_t key = ftok(name, 0);
    if(key == -1) {
    	std::cerr << "Not able to create key" << std::endl;
    }
    return key;
}

static char** strSplit(char* str, const char delimiter) {
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
    cout << str << endl;
    
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

static vector<string> strSplit(string str, char delimiter) {
    vector<string> split;
    char* cstr = (char*) malloc(str.size() - 1);
    strcpy(cstr, str.c_str());
    char** csplit = strSplit(cstr, delimiter);
    
    if(csplit) {
        int i;
        for(i = 0; *(csplit + i); i++) {
            char* next = *(csplit + i);
            split.push_back(string(next));

            free(next);
        }
        free(csplit);
    }

    delete[] cstr;
    return split;
}
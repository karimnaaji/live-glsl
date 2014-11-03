#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>

void redefineSignal(int sig, void (*handler)(int)) {
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

key_t createKey(const char* name) {
    key_t key = ftok(name, 0);
    if(key == -1) {
    	std::cerr << "Not able to create key" << std::endl;
    }
    return key;
}


#endif

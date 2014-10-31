#ifndef UTILS_H
#define UTILS_H

#include <iostream>

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

#endif

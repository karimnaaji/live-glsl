#pragma once

#include <string>
#include <stdint.h>

struct Arguments {
    std::string Input;
    std::string Output;
    uint32_t Width {800};
    uint32_t Height {600};
    bool EnableIni {true};
};

bool ArgumentsParse(int argc, const char** argv, Arguments& args);

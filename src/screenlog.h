#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "utils.h"
#include "graphics.h"
#include "glfontstash.h"

#define ARIAL "/Library/Fonts/Arial.ttf"

class ScreenLog {

public:
    ScreenLog(const std::string& fontFile, int size);
    ~ScreenLog();

    void render();

    ScreenLog& operator<< (std::string& s);
    ScreenLog& operator<< (std::string* s);
    void clear();

private:
    FONScontext* fs;
    FONSeffectType effect;
    int fontSize;
    int font;
    bool bufferedLog;
    std::vector<fsuint> textDisplay;
    std::string log;

};
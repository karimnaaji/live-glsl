#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "utils.h"
#include "graphics.h"
#include "glfontstash.h"

#define ARIAL "/Library/Fonts/Arial.ttf"
#define FONT_SIZE 25

class Log {

public:
    virtual Log& operator<< (std::string& s) = 0;
    virtual Log& operator<< (std::string* s) = 0;
    virtual Log& operator<< (const char* c) = 0;
    virtual void clear() = 0;

};

class ScreenLog : public Log {

public:
    static ScreenLog& Instance();
    ~ScreenLog();

    void render(bool clear);

    ScreenLog& operator<< (std::string& s) override;
    ScreenLog& operator<< (std::string* s) override;
    ScreenLog& operator<< (const char* c) override;

    void clear() override;

private:
    void unbuffer();
    ScreenLog();

    FONScontext* fs;
    FONSeffectType effect;
    int font;
    bool bufferedLog;
    std::vector<fsuint> textDisplay;
    std::string log;

};

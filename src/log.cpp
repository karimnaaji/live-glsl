#include "log.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"

ScreenLog& ScreenLog::Instance() {
    static ScreenLog instance;
    return instance;
}

ScreenLog::ScreenLog() {
    fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);

    font = fonsAddFont(fs, "sans", ARIAL);

    if(font == FONS_INVALID) {
        std::cerr << "Could not add font" << std::endl;
        glfonsDelete(fs);

        fs = NULL;
    } else {
        effect = FONS_EFFECT_NONE;
        
        fonsSetSize(fs, FONT_SIZE);
        fonsSetFont(fs, font);
    }

    glfonsUpdateViewport(fs);
}

ScreenLog::~ScreenLog() {
    unbuffer();
    glfonsDelete(fs);
}

void ScreenLog::render(bool clear) {
    if(log.compare("") == 0) {
        if(bufferedLog) {
            for(auto id : textDisplay) {
                glfonsUnbufferText(fs, id);
            }

            textDisplay.clear();
            bufferedLog = false;
        }
    } else {
        if(!bufferedLog) {
            fsuint id;

            for(auto str : strSplit(log, '\n')) {
                glfonsBufferText(fs, str.c_str(), &id, effect);
                textDisplay.push_back(id);   
            }

            bufferedLog = true;
        }

        if(clear) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        float yOffset = FONT_SIZE + FONT_SIZE / 4.0; 
        float yPad = yOffset;

        glfonsPushMatrix(fs);
        glfonsTranslate(fs, FONT_SIZE / 2.0, 0.0);

        for(auto id : textDisplay) {
            glfonsTranslate(fs, 0.0, yPad);
            glfonsDrawText(fs, id); 
        } 

        glfonsPopMatrix(fs);

        glDisable(GL_BLEND);
    }
}

void ScreenLog::unbuffer() {
    for(auto id : textDisplay) {
        glfonsUnbufferText(fs, id);
    }
    textDisplay.clear();
}

void ScreenLog::clear() {
    log = "";
    unbuffer();
    bufferedLog = false;
}

ScreenLog& ScreenLog::operator<< (const char* c) {
    std::string s(c);
    return *this << s;
}

ScreenLog& ScreenLog::operator<< (std::string& s) {
    log += s;
    return *this;
}

ScreenLog& ScreenLog::operator<< (std::string* s) {
    log += *s;
    return *this;
}
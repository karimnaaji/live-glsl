#include "screenlog.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"

ScreenLog::ScreenLog(const std::string& fontFile, int size) : fontSize(size) {
    fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);

    font = fonsAddFont(fs, "sans", fontFile.c_str());

    if(font == FONS_INVALID) {
        std::cerr << "Could not add font" << std::endl;
        glfonsDelete(fs);

        fs = NULL;
    } else {
        effect = FONS_EFFECT_NONE;
        
        fonsSetSize(fs, fontSize);
        fonsSetFont(fs, font);
    }

    glfonsUpdateViewport(fs);
}

ScreenLog::~ScreenLog() {
    if(bufferedLog) {
        for(auto id : textDisplay) {
            glfonsUnbufferText(fs, id);
        }
    }
    glfonsDelete(fs);
}

void ScreenLog::render() {
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
            for(auto str : strSplit(log, '\n')) {
                fsuint id;
                glfonsBufferText(fs, str.c_str(), &id, effect);
                textDisplay.push_back(id);   
            }
            bufferedLog = true;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        float yOffset = fontSize + fontSize / 4.0; 
        float yPad = yOffset;

        glfonsPushMatrix(fs);
        glfonsTranslate(fs, fontSize / 2.0, 0.0);

        for(auto id : textDisplay) {
            glfonsTranslate(fs, 0.0, yPad);
            glfonsDrawText(fs, id); 
        } 

        glfonsPopMatrix(fs);

        glDisable(GL_BLEND);
    }
}

void ScreenLog::clear() {
    log = "";
}

ScreenLog& ScreenLog::operator<< (std::string& s) {

    return *this;
}

ScreenLog& ScreenLog::operator<< (std::string* s) {

    return *this;
}
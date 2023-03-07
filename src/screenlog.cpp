#include "screenlog.h"

#include "arial.ttf.h"
#include "utils.h"

#include <assert.h>

#define GLFONTSTASH_IMPLEMENTATION
#include <glfontstash.h>

#define FONT_SIZE 20

ScreenLog ScreenLogCreate(float pixel_density) {
    ScreenLog screen_log;

    screen_log.LogBuffered = false;
    screen_log.FontContext = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);

    int font = fonsAddFontMem(screen_log.FontContext, "sans", arial_ttf, ARRAY_LENGTH(arial_ttf), 0);

    assert(font != FONS_INVALID &&  "Could not load font Arial");

    fonsSetSize(screen_log.FontContext, FONT_SIZE * pixel_density);
    fonsSetFont(screen_log.FontContext, font);

    glfonsUpdateViewport(screen_log.FontContext);

    return screen_log;
}

void ScreenLogDestroy(ScreenLog& screen_log) {
    for (auto id : screen_log.TextHandles) {
        glfonsUnbufferText(screen_log.FontContext, id);
    }

    glfonsDelete(screen_log.FontContext);
}

void ScreenLogRender(ScreenLog& screen_log, float pixel_density) {
    if (screen_log.Log.empty()) {
        if (screen_log.LogBuffered) {
            for(auto id : screen_log.TextHandles) {
                glfonsUnbufferText(screen_log.FontContext, id);
            }

            screen_log.TextHandles.clear();
            screen_log.LogBuffered = false;
        }
    } else {
        if (!screen_log.LogBuffered) {
            fsuint id;
            for (auto str : SplitString(screen_log.Log, '\n')) {
                glfonsBufferText(screen_log.FontContext, str.c_str(), &id, FONS_EFFECT_NONE);
                screen_log.TextHandles.push_back(id);
            }
            screen_log.LogBuffered = true;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfonsUpdateViewport(screen_log.FontContext);
        float y_offset = FONT_SIZE * pixel_density + FONT_SIZE * pixel_density * 0.25f;
        glfonsPushMatrix(screen_log.FontContext);
        glfonsTranslate(screen_log.FontContext, FONT_SIZE * pixel_density * 0.5f, 0.0f);

        for (auto id : screen_log.TextHandles) {
            glfonsTranslate(screen_log.FontContext, 0.0f, y_offset);
            glfonsDrawText(screen_log.FontContext, id);
        }

        glfonsPopMatrix(screen_log.FontContext);
    }
}

void ScreenLogRenderFrameStatus(ScreenLog& screen_log, uint32_t screen_width, bool sixty_fps, float pixel_density) {
    char buffer[32];
    sprintf(buffer, "■");
    static fsuint id = 0;

    if (id != 0) {
        glfonsUnbufferText(screen_log.FontContext, id);
    }

    glfonsBufferText(screen_log.FontContext, buffer, &id, FONS_EFFECT_NONE);
    glfonsUpdateViewport(screen_log.FontContext);

    if (sixty_fps) {
        glfonsSetColor(screen_log.FontContext, 0.21, 1.0, 0.74, 1.0);
    } else {
        glfonsSetColor(screen_log.FontContext, 1.0, 0.0, 0.0, 1.0);
    }

    glfonsPushMatrix(screen_log.FontContext);
    glfonsTranslate(screen_log.FontContext, screen_width - FONT_SIZE * pixel_density, FONT_SIZE * pixel_density);
    glfonsDrawText(screen_log.FontContext, id);
    glfonsPopMatrix(screen_log.FontContext);
    glfonsSetColor(screen_log.FontContext, 1.0, 1.0, 1.0, 1.0);
}

void ScreenLogClear(ScreenLog& screen_log) {
    screen_log.Log = "";

    for (auto id : screen_log.TextHandles) {
        glfonsUnbufferText(screen_log.FontContext, id);
    }

    screen_log.TextHandles.clear();
    screen_log.LogBuffered = false;
}

void ScreenLogBuffer(ScreenLog& screen_log, const char* c) {
    screen_log.Log += std::string(c);
}

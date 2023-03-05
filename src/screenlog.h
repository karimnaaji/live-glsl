#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glfontstash.h>

#include <vector>
#include <string>

struct ScreenLog {
    FONScontext* FontContext;
    bool LogBuffered;
    std::vector<fsuint> TextHandles;
    std::string Log;
};

ScreenLog ScreenLogCreate(float pixel_density);
void ScreenLogDestroy(ScreenLog& screen_log);
void ScreenLogRender(ScreenLog& screen_log, float pixel_density);
void ScreenLogRenderFrameStatus(ScreenLog& screen_log, uint32_t screen_width, bool sixty_fps, float pixel_density);
void ScreenLogClear(ScreenLog& screen_log);
void ScreenLogBuffer(ScreenLog& screen_log, const char* c);

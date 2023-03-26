#pragma once

#include "gui.h"
#include "arguments.h"
#include "renderpass.h"
#include "filewatcher.h"

#include <glad/gl.h>
#include <atomic>

struct LiveGLSL {
    std::vector<GUIComponent> GUIComponents;
    std::vector<RenderPass> RenderPasses;
    GLFWwindow* GLFWWindowHandle;
    Arguments Args;
    HFileWatcher FileWatcher;
    HGUI GUI;
    std::string ShaderPath;
    std::string BasePath;
    int WindowWidth;
    int WindowHeight;
    GLuint VertexBufferId;
#ifndef EMSCRIPTEN
    GLuint VaoId;
#endif
    float PixelDensity;
    std::atomic<bool> ShaderFileChanged;
    bool ShaderCompiled;
    bool IsContinuousRendering;
    std::string CanvasId = "canvas";
};

LiveGLSL* LiveGLSLCreate(const Arguments& args);
void LiveGLSLDestroy(LiveGLSL* live_glsl);
int LiveGLSLRender(LiveGLSL* live_glsl);

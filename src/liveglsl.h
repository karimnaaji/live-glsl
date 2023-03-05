#pragma once

#include "gui.h"
#include "arguments.h"
#include "renderpass.h"
#include "filewatcher.h"

#include <glad/glad.h>
#include <atomic>

struct LiveGLSL {
    std::vector<GUIComponent> GUIComponents;
    std::vector<RenderPass> RenderPasses;
    GLFWwindow* GLFWWindowHandle;
    Arguments Args;
	ScreenLog ScreenLogInstance;
	HFileWatcher FileWatcher;
    std::string ShaderPath;
    std::string BasePath;
    int WindowWidth;
    int WindowHeight;
    GLuint VertexBufferId;
    GLuint VaoId;
    float PixelDensity;
	std::atomic<bool> ShaderFileChanged;
    bool ShaderCompiled;
    bool IsContinuousRendering;
};

LiveGLSL* LiveGLSLCreate(const Arguments& args);
void LiveGLSLDestroy(LiveGLSL* live_glsl);
int LiveGLSLRender(LiveGLSL* live_glsl);
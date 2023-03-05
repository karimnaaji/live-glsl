#pragma once

#include <vector>
#include <string>

#include <glad/glad.h>

#include "shader.h"

struct RenderPass {
    ShaderProgram Program;
    std::string ShaderSource;
    std::string Input;
    std::string Output;
    bool IsMain {false};
    uint32_t Width {0};
    uint32_t Height {0};
    GLuint FBO {0};
    GLuint Texture {0};
};

void RenderPassDestroy(std::vector<RenderPass>& render_passes);
bool RenderPassCreate(std::vector<RenderPass>& render_passes, ScreenLog& screen_log);
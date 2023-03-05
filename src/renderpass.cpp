#include "renderpass.h"

static const GLchar* DefaultVertexShader = R"END(
in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
)END";

void RenderPassDestroy(std::vector<RenderPass>& render_passes) {
    for (auto& render_pass : render_passes) {
        ShaderProgramDetach(render_pass.Program);

        if (render_pass.Texture != 0) {
            glDeleteTextures(1, &render_pass.Texture);
        }

        if (render_pass.FBO != 0) {
            glDeleteFramebuffers(1, &render_pass.FBO);
        }
    }

    render_passes.clear();
}

bool RenderPassCreate(std::vector<RenderPass>& render_passes, ScreenLog& screen_log) {
    for (auto& render_pass : render_passes) {
        if (!ShaderProgramCreate(render_pass.Program, render_pass.ShaderSource, DefaultVertexShader, screen_log)) {
            return false;
        }

        if (!render_pass.IsMain) {
            glGenFramebuffers(1, &render_pass.FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, render_pass.FBO);
            glGenTextures(1, &render_pass.Texture);
            glBindTexture(GL_TEXTURE_2D, render_pass.Texture);
            assert(render_pass.Width != 0);
            assert(render_pass.Height != 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, render_pass.Width, render_pass.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_pass.Texture, 0);
            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    return true;
}
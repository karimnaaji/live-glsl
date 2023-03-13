#include "renderpass.h"

#include <assert.h>
#include <stb/stb_image.h>

static const GLchar* DefaultVertexShader = R"END(
in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
)END";

void RenderPassDestroy(std::vector<RenderPass>& render_passes) {
    for (auto& render_pass : render_passes) {
        ShaderProgramDetach(render_pass.Program);

        if (render_pass.TextureId != 0) {
            glDeleteTextures(1, &render_pass.TextureId);
        }

        if (render_pass.FBO != 0) {
            glDeleteFramebuffers(1, &render_pass.FBO);
        }

        for (auto& texture : render_pass.Textures) {
            if (texture.Id != 0) {
                glDeleteTextures(1, &texture.Id);
                stbi_image_free(texture.Data);
            }
        }
    }

    render_passes.clear();
}

bool RenderPassCreate(std::vector<RenderPass>& render_passes, std::string& error) {
    for (auto& render_pass : render_passes) {
        if (!ShaderProgramCreate(render_pass.Program, render_pass.ShaderSource, DefaultVertexShader, error)) {
            return false;
        }

        if (!render_pass.IsMain) {
            glGenFramebuffers(1, &render_pass.FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, render_pass.FBO);

            glGenTextures(1, &render_pass.TextureId);
            glBindTexture(GL_TEXTURE_2D, render_pass.TextureId);

            assert(render_pass.Width != 0);
            assert(render_pass.Height != 0);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, render_pass.Width, render_pass.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_pass.TextureId, 0);

            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        for (Texture& texture : render_pass.Textures) {
            glGenTextures(1, &texture.Id);
            glBindTexture(GL_TEXTURE_2D, texture.Id);

            assert(texture.Width != 0);
            assert(texture.Height != 0);
            assert(texture.Data);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width, texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.Data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    return true;
}
#pragma once

#include <glad/gl.h>
#include <string.h>
#include <string>

struct ShaderProgram {
    GLuint Handle;
    GLuint FragmentShaderHandle;
    GLuint VertexShaderHandle;

    ShaderProgram() {
        memset(this, 0x0, sizeof(ShaderProgram));
    }
};

void ShaderProgramDestroy(ShaderProgram& shader_program);
GLuint ShaderProgramCompile(const std::string src, GLenum type, std::string& error);
bool ShaderProgramCreate(ShaderProgram& shader_program, const std::string& fragment_source, const std::string& vertex_source, std::string& error);
void ShaderProgramDetach(const ShaderProgram& shader_program);
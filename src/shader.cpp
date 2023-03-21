#include  "shader.h"

void ShaderProgramDestroy(ShaderProgram& shader_program) {
    if (shader_program.Handle) {
        glDeleteProgram(shader_program.Handle);
    }

    if (shader_program.VertexShaderHandle) {
        glDeleteShader(shader_program.VertexShaderHandle);
    }

    if (shader_program.FragmentShaderHandle) {
        glDeleteShader(shader_program.FragmentShaderHandle);
    }
}

GLuint ShaderProgramCompile(const std::string src, GLenum type, std::string& error) {
    GLint compile_status;
    GLuint shader = glCreateShader(type);
    const GLchar* source = (const GLchar*) src.c_str();

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if (!compile_status) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        
        char info[8096];
        glGetShaderInfoLog(shader, length, NULL, info);
        
        error = info;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool ShaderProgramCreate(ShaderProgram& shader_program, const std::string& fragment_source, const std::string& vertex_source, std::string& error) {
#ifdef EMSCRIPTEN
    std::string shader_prelude = "";
#else 
    std::string shader_prelude = "#version 150\n";
#endif

    shader_program.VertexShaderHandle = ShaderProgramCompile(shader_prelude + vertex_source, GL_VERTEX_SHADER, error);
    shader_program.FragmentShaderHandle = ShaderProgramCompile(shader_prelude + fragment_source, GL_FRAGMENT_SHADER, error);

    if (!shader_program.FragmentShaderHandle || !shader_program.VertexShaderHandle) {
        ShaderProgramDestroy(shader_program);
        return false;
    }

    shader_program.Handle = glCreateProgram();

    glAttachShader(shader_program.Handle, shader_program.VertexShaderHandle);
    glAttachShader(shader_program.Handle, shader_program.FragmentShaderHandle);

    glLinkProgram(shader_program.Handle);

    glDeleteShader(shader_program.FragmentShaderHandle);
    glDeleteShader(shader_program.VertexShaderHandle);

    GLint is_linked;
    glGetProgramiv(shader_program.Handle, GL_LINK_STATUS, &is_linked);

    if (is_linked == GL_FALSE) {
        error = "Error linking program";
        ShaderProgramDestroy(shader_program);
        return false;
    }

    return true;
}

void ShaderProgramDetach(const ShaderProgram& shader_program) {
    return;
    if (shader_program.VertexShaderHandle) {
        glDetachShader(shader_program.VertexShaderHandle, GL_VERTEX_SHADER);
    }

    if (shader_program.FragmentShaderHandle) {
        glDetachShader(shader_program.FragmentShaderHandle, GL_FRAGMENT_SHADER);
    }
}

#ifndef SHADER_H
#define SHADER_H

#include "vec3.h"
#include <iostream>
#include <GL/glew.h>

/*class Shader {
private:
public:
    Shader() : v(Vec3(10.0)) {}
    Vec3 v;
};

int main() {
    Shader s;
    std::cout << s.v.x << std::endl;
    return 0;
}*/
/*
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cassert>
#include <map>
#include <GL/glew.h>

#include "bufferattribute.h"

using namespace std;
using namespace glm;

class Shader {
public:
    Shader(string shaderName);
    ~Shader(void);

    GLuint programId() const;
    void init(const map<string, BufferAttribute>& attributes);
    string getName() const;
    bool isInUse() const;
    void use() const;
    void release() const;

    void sendUniform(string name, float value) const;
    void sendUniform(string name, int value) const;
    void sendUniform(string name, const mat4& mat) const;
    void sendUniform(string name, const mat3& mat) const;
    void sendUniform(string name, const vec2& vec) const;
    void sendUniform(string name, const vec3& vec) const;
    void sendUniform(string name, const vec4& vec) const;

    void setAttribute(BufferAttribute attribute, const string& name);

private:
    void link();
    GLuint createShader(GLenum type, const string& file);
    void loadShaderSource(const string& file, string* into) const;
    void setDefaultAttributes(const map<string, BufferAttribute>& attributes);
    GLint uniform(string uniformName) const;

    GLuint fragment;
    GLuint vertex;
    GLuint program;
    bool linked;
    string name;
};*/


#endif

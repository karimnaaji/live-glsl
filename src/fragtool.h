#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include "graphics.h"
#include "filewatcher.h"
#include "default_vert.h"
#include "utils.h"
#include "shader.h"
#include "log.h"

using namespace std;

class FragTool {

public:
    void watchingThread();
    void renderingThread();
    void setChildProcess(pid_t pid);
    void setParentProcess(pid_t pid);
    void setFragShaderPath(const string& fragShaderPath);

    friend void handleResize(GLFWwindow* window, int w, int h);
    friend void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void watcherCallback();

    void fragmentHasChanged();
    void destroy();

private:
    void render();
    void init();
    void initShader();

    void handleError(const string& message, int exitStatus);
    bool linkShaderToProgram(GLuint program, const GLchar* source, GLenum type);
    bool loadShaderSource(const string& path, string* into);
    GLuint compileShader(const GLchar* src, GLenum type);
    void printShaderInfoLog(GLuint shader);

    GLFWwindow* window;
    bool fragHasChanged;

    GLuint vbo;
    GLint posAttrib;
    string fragShaderPath;

    pid_t childProcess;
    pid_t parentProcess;

    FileWatcher watcher;

    int width;
    int height;

    Shader shader;
    
};

extern void handleResize(GLFWwindow* window, int w, int h);
extern void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);
extern void watcherCallback();
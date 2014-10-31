#ifndef FRAGTOOL_H
#define FRAGTOOL_H

#include <iostream>
#include <fstream>
#include <memory>
#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include "filewatcher.h"
#include "default_vert.h"

using namespace std;

class FragTool {
public:
    static unique_ptr<FragTool> GetInstance();
    
    ~FragTool();
    void watchingThread();
    void renderingThread();

    void initShader();
    bool linkShaderToProgram(GLuint program, const GLchar* source, GLenum type);
    bool loadShaderSource(const string& path, string* into);
    GLuint compileShader(const GLchar* src, GLenum type);

    void printShaderInfoLog(GLuint shader);
    void fragmentHasChanged();
    void watcherCallback();

    void setChildProcess(pid_t pid);
    void setParentProcess(pid_t pid);
    void setFragShaderPath(const string& fragShaderPath);

    static void handleResize(GLFWwindow* window, int w, int h);
    static void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    FragTool();

    void handleError(const string& message, int exitStatus);
    void render();

    GLuint vbo;
    
    GLint shaderProgram;
    GLint posAttrib;

    GLuint fragmentId;
    GLuint vertexId;

    string fragShaderPath;

    pid_t childProcess;
    pid_t parentProcess;

    FileWatcher* watcher;
    bool fragHasChanged;

    int width;
    int height;
};

void watcherCallbackWrapper();

static const GLfloat vertices[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
};

#endif

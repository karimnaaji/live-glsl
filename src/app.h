#pragma once

#include <iostream>
#include <memory>
#include <mutex>

#include "GLFW/glfw3.h"
#include "shader.h"

using namespace std;

class App {

public:
    App();
    App(const std::string& fragShaderPath, std::shared_ptr<std::mutex> mutex);
    ~App();

    bool initGL();
    void renderLoop();

    friend void handleResize(GLFWwindow* window, int w, int h);
    friend void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);

    bool fragHasChanged;

private:
    void renderFrame();
    bool initShader();

    GLFWwindow* window;
    Shader shader;
    GLuint vbo;
    GLint posAttrib;
    string fragShaderPath;

    std::shared_ptr<std::mutex> mutex;

    int width;
    int height;
};

extern void handleResize(GLFWwindow* window, int w, int h);
extern void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);


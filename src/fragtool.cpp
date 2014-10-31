#include "fragtool.h"

unique_ptr<FragTool> FragTool::GetInstance() {
    static unique_ptr<FragTool> instance(new FragTool());
    return move(instance);
}

FragTool::FragTool() {
    fragHasChanged = false;
}

FragTool::~FragTool() {
    delete watcher;

    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
}

bool FragTool::linkShaderToProgram(GLuint program, const GLchar* source, GLenum type) {
    switch(type) {
        case GL_VERTEX_SHADER: {
            vertexId = compileShader(source, GL_VERTEX_SHADER);
            glAttachShader(program, vertexId);
        }
        break;
        case GL_FRAGMENT_SHADER: {
            fragmentId = compileShader(source, GL_FRAGMENT_SHADER);
            if(fragmentId == -1) {
                return false;
            }
            glAttachShader(program, fragmentId);
        }
        break;
    }

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if(!linkStatus) {
        printShaderInfoLog(program);
        glDeleteProgram(program);
        return false;
    }

    return true;
}

void FragTool::printShaderInfoLog(GLuint shader) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        char* log = (char*)malloc(sizeof(char) * length);
        glGetShaderInfoLog(shader, length, NULL, log);
        cerr << "Log: " << log << endl;
        free(log);
    }
}

GLuint FragTool::compileShader(const GLchar* src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(!status) {
        printShaderInfoLog(shader);
        glDeleteShader(shader);
        return -1;
    }
    return shader;
}

bool FragTool::loadShaderSource(const string& path, string* into) {
    ifstream file;
    string buffer;

    file.open(path.c_str());

    if(!file.is_open()) {
        return false;
    }

    while(!file.eof()) {
        getline(file, buffer);
        (*into) += buffer + "\n";
    }

    file.close();
    return true;
}

void FragTool::initShader() {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    shaderProgram = glCreateProgram();

    string fragSource;
    loadShaderSource(fragShaderPath, &fragSource);

    linkShaderToProgram(shaderProgram, vertexShaderSrc, GL_VERTEX_SHADER);
    linkShaderToProgram(shaderProgram, fragSource.c_str(), GL_FRAGMENT_SHADER);

    posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);
}

void FragTool::fragmentHasChanged() {
    fragHasChanged = true;
}

void FragTool::watcherCallback() {
    kill(parentProcess, SIGALRM);
}

void FragTool::handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    unique_ptr<FragTool> instance = FragTool::GetInstance();
    switch (key) {
        case 256:
            kill(instance->childProcess, SIGALRM);
            exit(0);
    }
}

void FragTool::handleResize(GLFWwindow* window, int w, int h) {
    unique_ptr<FragTool> instance = FragTool::GetInstance();
    instance->width = w;
    instance->height = h;
    glViewport(0, 0, w, h);
}

void FragTool::setChildProcess(pid_t pid) {
    childProcess = pid;
}

void FragTool::setParentProcess(pid_t pid) {
    parentProcess = pid;
}

void FragTool::setFragShaderPath(const string& shaderPath) {
    fragShaderPath = shaderPath;
}

void FragTool::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glUniform2f(glGetUniformLocation(shaderProgram, "resolution"), width, height);
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), glfwGetTime());

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glEnableVertexAttribArray(0);
}

void FragTool::renderingThread() {
    int width = 800;
    int height = 600;
    GLFWwindow* window;

    if(!glfwInit())
        handleError("GLFW init failed", -1);

    window = glfwCreateWindow(width, height, "fragtool", NULL, NULL);

    if(!window) {
        glfwTerminate();
        handleError("GLFW create window failed", -1);
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if(err != GLEW_OK) {
        cerr << glewGetErrorString(err) << endl;
        handleError("GlEW init failed", -1);
    }

    glfwSetWindowSizeCallback(window, handleResize);
    glfwSetKeyCallback(window, handleKeypress);

    initShader();

    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        if(fragHasChanged) {
            string fragSource;
            if(loadShaderSource(fragShaderPath, &fragSource)) {
                glDetachShader(shaderProgram, fragmentId);
                linkShaderToProgram(shaderProgram, fragSource.c_str(), GL_FRAGMENT_SHADER);
                fragHasChanged = false;
            }
        }
        render();
        glfwPollEvents();
    }

    glfwTerminate();
}

void FragTool::handleError(const string& message, int exitStatus) {
    cerr << "ABORT: "<< message << endl;
    kill(childProcess, SIGKILL);
    exit(exitStatus);
}

void FragTool::watchingThread() {
    char* s = new char[1024];
    realpath(fragShaderPath.c_str(), s);
    string absolutePath(s);
    watcher = new FileWatcher(absolutePath, &watcherCallbackWrapper);
    watcher->startWatching();
}

void watcherCallbackWrapper() {
    FragTool::GetInstance()->watcherCallback();
}
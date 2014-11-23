#include "fragtool.h"

void FragTool::init() {
    shader.log = &(ScreenLog::Instance());
    fragHasChanged = false;
    initShader();
}

void FragTool::destroy() {
    glDeleteBuffers(1, &vbo);
}

void FragTool::initShader() {
    float vertices[12];
    
    quad(vertices);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    string fragSource;

    if(!loadFromPath(fragShaderPath, &fragSource)) {
        return;
    }

    shader.build(fragSource, vertexShader);

    posAttrib = glGetAttribLocation(shader.getProgram(), "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);
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

    shader.use();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    shader.sendUniform("resolution", width, height);
    shader.sendUniform("time", glfwGetTime());

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void FragTool::fragmentHasChanged() {
    fragHasChanged = true;
}

void FragTool::renderingThread() {
    width = 800;
    height = 600;

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

    init();

    glClearColor(56.0/255, 101.0/255, 190.0/255, 1);

    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if(fragHasChanged) {
            string fragSource;

            if(loadFromPath(fragShaderPath, &fragSource)) {
                shader.detach(GL_FRAGMENT_SHADER | GL_VERTEX_SHADER);
                shader.build(fragSource, vertexShader);
                fragHasChanged = false;
            }
        }

        render(); 
        ScreenLog::Instance().render(true); 
        //ScreenLog::Instance().clear(); 

        glfwPollEvents();
    }

    glfwTerminate();
}

void FragTool::handleError(const string& message, int exitStatus) {
    cerr << "ABORT: "<< message << endl;
    exit(exitStatus);
}

void FragTool::watchingThread() {
    char s[1024];
    realpath(fragShaderPath.c_str(), s);

    string absolutePath(s);

    watcher = FileWatcher(absolutePath, &watcherCallback);
    watcher.startWatching();
}

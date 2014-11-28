#include "fragtool.h"

void FragTool::init() {
    initGL();
    shader.log = &(ScreenLog::Instance());
    fragHasChanged = false;
    initShader();
}

void FragTool::destroy() {
    if(hasSound) {
        channel->stop();
        sound->release();
        system->release();
    }
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
        std::cerr << "Unable to load " << fragShaderPath << std::endl;
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

void FragTool::loadSoundSource(const string& path) {
    hasSound = true;
    soundPath = path;
    FMOD_RESULT result;

    FMOD::System_Create(&system);
    system->init(32, FMOD_INIT_NORMAL,0);
    system->createSound(soundPath.c_str(), FMOD_HARDWARE, 0, &sound);
    system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
    channel->setMode(FMOD_LOOP_NORMAL);
    channel->setLoopCount(-1);
}

void FragTool::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if(hasSound) {
        float spectrum[256];
        float wavedata[256];
        system->getWaveData(wavedata, 256, 0);
        system->getSpectrum(spectrum, 256, 0, FMOD_DSP_FFT_WINDOW_TRIANGLE);

        shader.sendUniform("wave", 256, wavedata);
        shader.sendUniform("spectrum", 256, spectrum);
    }

    shader.sendUniform("resolution", width, height);
    shader.sendUniform("time", glfwGetTime());

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void FragTool::fragmentHasChanged() {
    fragHasChanged = true;
}

void FragTool::initGL() {
    width = 800;
    height = 600;

    if(!glfwInit()) {
        handleError("GLFW init failed", -1);
    }

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
}

void FragTool::renderingThread() {
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

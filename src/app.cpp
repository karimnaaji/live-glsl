#include "app.h"

App::App() {}

App::App(const std::string& fragPath, std::shared_ptr<std::mutex> mtx)
: fragHasChanged(false), fragShaderPath(fragPath), mutex(mtx) {}

App::~App() {
    if(hasSound) {
        channel->stop();
        sound->release();
        system->release();
    }

    if(vbo != 0) {
        glDeleteBuffers(1, &vbo);
    }
}

bool App::initShader() {
    string fragSource;

    if(!loadFromPath(fragShaderPath, &fragSource)) {
        std::cerr << "Unable to load " << fragShaderPath << std::endl;
        return false;
    }

    if(!shader.build(fragSource, vertexShader)) {
        return false;
    }

    float vertices[12];
    quad(vertices);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    posAttrib = glGetAttribLocation(shader.getProgram(), "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    shader.log = &(ScreenLog::Instance());

    return true;
}

void App::loadSoundSource(const string& path) {
    hasSound = true;
    soundPath = path;
    FMOD_RESULT result;

    result = FMOD::System_Create(&system);

    if(result != FMOD_OK) {
        std::cerr << "FMOD error! (" << result << ")" << std::endl;
        return;
    }

    system->init(32, FMOD_INIT_NORMAL,0);
    system->createSound(soundPath.c_str(), FMOD_HARDWARE, 0, &sound);
    system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
    channel->setMode(FMOD_LOOP_NORMAL);
    channel->setLoopCount(-1);
}

void App::renderFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if(hasSound) {
        float spectrum[256];
        float wavedata[256];
        system->getWaveData(wavedata, 256, 0);
        system->getSpectrum(spectrum, 256, 0, FMOD_DSP_FFT_WINDOW_BLACKMANHARRIS);

        shader.sendUniform("wave", 256, wavedata);
        shader.sendUniform("spectrum", 256, spectrum);
    }

    shader.sendUniform("resolution", width, height);
    shader.sendUniform("time", glfwGetTime());

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool App::initGL() {
    width = 800;
    height = 600;

    if(!glfwInit()) {
        cerr << "GLFW init failed" << endl;
        return false;
    }

    window = glfwCreateWindow(width, height, "App", NULL, NULL);

    if(!window) {
        glfwTerminate();
        cerr << "GLFW create window failed" << endl;
        return false;
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowSizeCallback(window, handleResize);
    glfwSetKeyCallback(window, handleKeypress);

    glClearColor(0.21, 0.39, 0.74, 1.0);

    return initShader();
}

void App::renderLoop() {
    while(!glfwWindowShouldClose(window)) {
        string fragSource;

        if(fragHasChanged && loadFromPath(fragShaderPath, &fragSource)) {
            shader.detach(GL_FRAGMENT_SHADER | GL_VERTEX_SHADER);
            shader.build(fragSource, vertexShader);

            {
                std::lock_guard<std::mutex> lock(*mutex);
                fragHasChanged = false;
            }
        }

        renderFrame();
        ScreenLog::Instance().render(true);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
}


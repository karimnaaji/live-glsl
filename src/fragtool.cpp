#include "fragtool.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"

FragTool::FragTool() {
    fragHasChanged = false;
}

void FragTool::destroy() {
    if(bufferedLog) {
        for(auto id : textDisplay) {
            glfonsUnbufferText(fs, id);
        }
    }
    glfonsDelete(fs);
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

void FragTool::initFont() {
    fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
    font = fonsAddFont(fs, "sans", "/Library/Fonts/Arial.ttf");

    if(font == FONS_INVALID) {
        std::cerr << "Could not add font normal" << std::endl;
        glfonsDelete(fs);
        fs = NULL;
    } else {
        effect = FONS_EFFECT_NONE;
        
        fonsSetSize(fs, 25);
        fonsSetFont(fs, font);
    }
}

void FragTool::renderLog() {
    if(shaderLog.compare("") == 0) {
        if(bufferedLog) {
            for(auto id : textDisplay) {
                glfonsUnbufferText(fs, id);
            }
            textDisplay.clear();
            bufferedLog = false;
        }
    } else {
        if(!bufferedLog) {
            for(auto str : strSplit(shaderLog, '\n')) {
                fsuint id;
                glfonsBufferText(fs, str.c_str(), &id, effect);
                textDisplay.push_back(id);   
            }
            bufferedLog = true;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        float yOffset = 25.0;
        float yPad = yOffset;

        glfonsPushMatrix(fs);
        glfonsTranslate(fs, 10.0, 0.0);
        for(auto id : textDisplay) {
            glfonsTranslate(fs, 0.0, yPad);
            glfonsDrawText(fs, id); 
        } 
        glfonsPopMatrix(fs);
    }
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

    initShader();
    initFont();

    glfonsUpdateViewport(fs);
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
        //renderLog(); 

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

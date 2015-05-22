#include <iostream>
#include <thread>
#include <mutex>

#include "fragtool.h"
#include "filewatcher.h"
#include "utils.h"

static FragTool fragtool;
static FileWatcher watcher;
static bool quit = false;
static std::shared_ptr<std::mutex> mtx(new std::mutex());

void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
        case 256: // ESC
            glfwSetWindowShouldClose(fragtool.window, GL_TRUE);
    }
}

void handleResize(GLFWwindow* window, int w, int h) {
    fragtool.width = w;
    fragtool.height = h;

    glViewport(0, 0, w, h);
}

void watcherCallback() {
    std::lock_guard<std::mutex> lock(*mtx);
    fragtool.fragHasChanged = true;
}

void watchingThread(std::string fragShaderPath) {
    char s[1024];
    realpath(fragShaderPath.c_str(), s);

    string absolutePath(s);

    std::cout << "Watching fragment file source: " << s << std::endl;
    watcher = FileWatcher(absolutePath, &watcherCallback);

    watcher.startWatching(&quit);
}

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cerr << "Provide fragment shader argument" << std::endl;
        exit(-1);
    }

    std::string fragShaderPath = std::string(argv[1]);

    fragtool = FragTool(fragShaderPath, mtx);
    watcher = FileWatcher(fragShaderPath, &watcherCallback);

    if(argc > 2) {
        fragtool.loadSoundSource(std::string(argv[2]));
    }

    std::thread t(watchingThread, fragShaderPath);

    if(fragtool.initGL()) {
        fragtool.renderLoop();
    }

    quit = true;

    t.join();

    return 0;
}

#include <iostream>
#include "shader.h"

#include "GLFW/glfw3.h"

#include "filewatcher.h"

void callback() {
    printf("file changed\n");
}

void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			exit(0);
	}
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
}

void handleError(const string& message, int exitStatus) {
    cerr << "ABORT: "<< message << endl;
    exit(exitStatus);
}

void redefineSignal(int sig, void (*handler)(int)) {
    struct sigaction action;
    sigset_t set;

    sigemptyset(&set);
    action.sa_handler = handler;
    action.sa_flags = 0;
    action.sa_mask = set;

    if (sigaction(sig, &action, NULL) != 0) {
        cerr << "not able to redefine signal number" << sig << endl;
    }
}

void watchingThread(const char* argv, pid_t pid) {
    char* s = new char[1024];
    realpath(argv, s);
    string absolutePath(s);
    FileWatcher watcher(absolutePath, &callback);
    cout << pid << " start watching " << absolutePath << endl;
    watcher.startWatching();
}

void renderingThread(pid_t pid) {
    cout << pid << "start glfw window" << endl;
    GLFWwindow* window;
    int width = 800;
    int height = 600;

    if(!glfwInit())
        handleError("GLFW init failed", -1);

    window = glfwCreateWindow(width, height, "GLFW Window", NULL, NULL);

    if(!window) {
        glfwTerminate();
        handleError("GLFW create window failed", -1);
    }

    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

int main(int argc, char **argv) {
    if(argc < 2) {
        handleError("Provide fragment shader argument", -1);
    }

    pid_t pid = fork();

    switch(pid) {
        case 0:
            watchingThread(argv[1], pid);
            break;
        default:
            renderingThread(pid);
    }

    return 0;
}

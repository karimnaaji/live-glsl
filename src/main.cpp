#include <iostream>
#include <fstream>
#include <thread>

#include "fragtool.h"
#include "utils.h"

static FragTool* fragtool;

/*
 * Parent event signal handling
 */
void fileHasChanged(int sig) {
    cout << "main fileHasChanged" << sig << " sigalrm " << SIGALRM << endl;
    if(sig == SIGALRM) {
		fragtool->fragmentHasChanged();
    }
}

/*
 * Event callback when file has changed
 */
void watcherCallback() {
    kill(fragtool->parentProcess, SIGALRM);
}

void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
        case 256: // ESC
            exit(0);
    }
}

void handleResize(GLFWwindow* window, int w, int h) {
    fragtool->width = w;
    fragtool->height = h;

    glViewport(0, 0, w, h);
}

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cerr << "Provide fragment shader argument" << std::endl;
		exit(-1);
    }

    int shmId = shmget(IPC_PRIVATE, sizeof(FragTool), 0666);

    std::string fragShaderPath = string(argv[1]);

    pid_t parent = getpid();
    pid_t child = fork();

    fragtool = (FragTool *) shmat(shmId, NULL, 0);

    switch(child) {
        case -1: 
            exit(-1);

        case 0: {
            fragtool->watchingThread();
        }
        break;

        default: {
            fragtool->setFragShaderPath(fragShaderPath);

            fragtool->setChildProcess(child);
            fragtool->setParentProcess(parent);

            redefineSignal(SIGALRM, fileHasChanged);

            fragtool->renderingThread();

            kill(child, SIGKILL);
        }
    }

    shmctl(shmId, IPC_RMID, NULL);

    return 0;
}

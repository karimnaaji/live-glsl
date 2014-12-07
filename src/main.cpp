#include <iostream>

#include "fragtool.h"
#include "utils.h"

static FragTool* fragtool;

/*
 * Parent event signal handling
 */
void fileHasChanged(int sig) {
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
            glfwSetWindowShouldClose(fragtool->window, GL_TRUE);
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

    std::string fragShaderPath = std::string(argv[1]);

    pid_t parent = getpid();
    pid_t child = fork();

    // shared memory block
    fragtool = (FragTool *) shmat(shmId, NULL, 0);

    fragtool->setFragShaderPath(fragShaderPath);
    fragtool->setChildProcess(child);
    fragtool->setParentProcess(parent);

    switch(child) {
        case -1: {
            shmctl(shmId, IPC_RMID, NULL);
            exit(-1);
        }
            
        case 0: {
            fragtool->watchingThread();
            break;
        }

        default: {
            if(argc > 2) {
                fragtool->loadSoundSource(std::string(argv[2]));
            }

            fragtool->init();

            redefineSignal(SIGALRM, fileHasChanged);

            fragtool->renderingThread();

            kill(child, SIGKILL);
        }
    }
    
    fragtool->destroy();

    // remove shared memory
    shmctl(shmId, IPC_RMID, NULL);

    return 0;
}

#include <iostream>
#include <fstream>
#include <memory>

#include "fragtool.h"
#include "utils.h"

/*
 * Parent event signal handling
 */
void fileHasChanged(int sig) {
    std::unique_ptr<FragTool> fragtool = FragTool::GetInstance();
    if(sig == SIGALRM) {
		fragtool->fragmentHasChanged();
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cerr << "Provide fragment shader argument" << std::endl;
		exit(-1);
    }

    pid_t parent, child;
    std::unique_ptr<FragTool> fragtool = FragTool::GetInstance();
    std::string fragShaderPath = string(argv[1]);

    parent = getpid();
    child = fork();

    if(child < 0) {
		exit(-1);
	}

    switch(child) {
        case 0: {
            fragtool->watchingThread();
        }
        break;

        default: {
            fragtool->setChildProcess(child);
            fragtool->setParentProcess(parent);
            fragtool->setFragShaderPath(fragShaderPath);

            redefineSignal(SIGALRM, fileHasChanged);

            fragtool->renderingThread();

            kill(child, SIGALRM);
        }
    }

    return 0;
}

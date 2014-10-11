#include <iostream>
#include "shader.h"
#include "GLFW/glfw3.h"

#include "filewatcher.h"

void callback() {
    printf("file changed\n");
}

int main(int argc, char **argv) {
    if(argc < 2)
        exit(-1);

    char* s = new char[1024];
    realpath(argv[1], s);
    string absolutePath(s);
    FileWatcher watcher(absolutePath, &callback);
    cout << "start watching " << absolutePath << endl;
    watcher.startWatching();

    return 0;
}

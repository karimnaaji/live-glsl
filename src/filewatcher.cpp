#include "filewatcher.h"

FileWatcher::FileWatcher(const std::string& absolutePath, void (*cb)(void))
: file(absolutePath), callback(cb) {}

void FileWatcher::startWatching(bool* quit) {
    static int s_lastChange;
    struct stat st;
    stat(file.c_str(), &st);

    while(!*quit) {
        stat(file.c_str(), &st);
        if(st.st_mtime != s_lastChange) {
            processEvent();
            s_lastChange = st.st_mtime;
        }
        usleep(1000);
    }
}

void FileWatcher::processEvent() const {
    if(callback != NULL) {
        (*callback)();
    }
}

std::string FileWatcher::watchedFile() const {
    return file;
}


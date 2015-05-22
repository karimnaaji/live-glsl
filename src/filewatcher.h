#pragma once

#include <string>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>

class FileWatcher;

class FileWatcher {
public:
    FileWatcher() {};
    FileWatcher(const std::string& absolutePath, void (*cb)(void));
    void startWatching(bool* quit);
    std::string watchedFile() const;
    void processEvent() const;

private:
    std::string file;
    void (*callback)(void);
};


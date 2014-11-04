#pragma once

#include <string>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <iostream>

using namespace std;

class FileWatcher;

typedef struct {
    size_t len;
    size_t size;
    char **paths;
    FileWatcher* watcher;
} ctx_desc;

class FileWatcher {
public:
    FileWatcher(const string& absolutePath, void (*cb)(void));
    void startWatching();
    string watchedFile() const;
    void processEvent() const;
    
private:
    ctx_desc* ctxDesc;
    string file;
    void (*callback)(void);
};

void eventCallback(ConstFSEventStreamRef streamRef,
      void *ctx,
      size_t count,
      void *paths,
      const FSEventStreamEventFlags flags[],
      const FSEventStreamEventId ids[]);
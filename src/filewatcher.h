#pragma once

#include <stdint.h>

typedef void* HFileWatcher;
typedef void(*FFileWatcherCallback)(void*, const char*);

HFileWatcher FileWatcherCreate(FFileWatcherCallback callback, void* user_data, int64_t intervals_in_ms);
void FileWatcherDestroy(HFileWatcher);
void FileWatcherAddWatch(HFileWatcher, const char* watch);
void FileWatcherRemoveWatch(HFileWatcher, const char* watch);
void FileWatcherRemoveAllWatches(HFileWatcher);
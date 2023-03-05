#include "filewatcher.h"

#include <vector>
#include <string>
#include <map>
#include <atomic>
#include <thread>
#include <mutex>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#define MAX_PATH 8096

struct FileWatcher {
    void* UserData;
    FFileWatcherCallback Callback;
    std::vector<std::string> Watches;
    std::atomic<bool> ShouldQuit;
    std::thread WatcherThread;
    std::mutex Mutex;
    int64_t IntervalsInMS;
};

void FileWatcherThread(FileWatcher* watcher) {
    std::map<std::string, int> last_changed;
    while (!watcher->ShouldQuit) {
        std::vector<std::string> watches;
        {
            std::lock_guard<std::mutex> guard(watcher->Mutex);
            watches = watcher->Watches;
        }
        for (const auto& watch : watches) {
            char real_path[MAX_PATH];

#ifdef _WIN32
            DWORD length = GetFullPathNameA(watch.c_str(), MAX_PATH, real_path, nullptr);
            if (length == 0 || length >= MAX_PATH) {
                continue;
            }
            WIN32_FILE_ATTRIBUTE_DATA file_attributes;
            if (!GetFileAttributesExA(real_path, GetFileExInfoStandard, &file_attributes)) {
                continue;
            }
            int current_changed = file_attributes.ftLastWriteTime.dwLowDateTime;
#else
            realpath(watch.c_str(), real_path);
            struct stat st;
            stat(real_path, &st);
            int current_changed = st.st_mtime;
#endif

            if (current_changed != last_changed[watch]) {
                watcher->Callback(watcher->UserData, watch.c_str());
                last_changed[watch] = current_changed;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(watcher->IntervalsInMS));
    }
}

HFileWatcher FileWatcherCreate(FFileWatcherCallback callback, void* user_data, int64_t intervals_in_ms) {
    FileWatcher* watcher = new FileWatcher();
    watcher->UserData = user_data;
    watcher->ShouldQuit.store(false);
    watcher->WatcherThread = std::thread(FileWatcherThread, watcher);
    watcher->IntervalsInMS = intervals_in_ms;
    watcher->Callback = callback;
    return watcher;
}

void FileWatcherDestroy(HFileWatcher handle) {
    FileWatcher* watcher = (FileWatcher*)handle;
    watcher->ShouldQuit.store(true);
    watcher->WatcherThread.join();
    delete watcher;
}

void FileWatcherAddWatch(HFileWatcher handle, const char* watch) {
    FileWatcher* watcher = (FileWatcher*)handle;
    std::lock_guard<std::mutex> guard(watcher->Mutex);
    auto it = std::find(watcher->Watches.begin(), watcher->Watches.end(), watch);
    if (it == watcher->Watches.end()) {
        watcher->Watches.push_back(std::string(watch));
    }
}

void FileWatcherRemoveWatch(HFileWatcher handle, const char* watch) {
    FileWatcher* watcher = (FileWatcher*)handle;
    std::lock_guard<std::mutex> guard(watcher->Mutex);
    watcher->Watches.push_back(std::string(watch));
}

void FileWatcherRemoveAllWatches(HFileWatcher handle) {
    FileWatcher* watcher = (FileWatcher*)handle;
    std::lock_guard<std::mutex> guard(watcher->Mutex);
    watcher->Watches.clear();
}
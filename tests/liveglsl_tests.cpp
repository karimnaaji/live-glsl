#include "arguments.h"
#include "utils.h"
#include "filewatcher.h"
#include "utest.h"

#include <string.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#define T(b) EXPECT_TRUE(b)
#define TSTR(s0, s1) EXPECT_TRUE(0 == strcmp(s0,s1))

UTEST(arguments_tests, parse_0) {
    Arguments arguments;
    const char* args[] = {
        "liveglsl",
        "--input",
        "shader.frag",
        "--width",
        "300",
        "--height",
        "200"
    };
    T(ArgumentsParse(ARRAY_LENGTH(args), args, arguments));
    TSTR(arguments.Input.c_str(), "shader.frag");
    T(arguments.Width == 300);
    T(arguments.Height == 200);
}

UTEST(arguments_tests, parse_1) {
    Arguments arguments;
    const char* args[] = {
        "liveglsl",
        "shader.frag",
    };
    T(!ArgumentsParse(ARRAY_LENGTH(args), args, arguments));
}

void OnFileChanged(void* user_data, const char* file_path) {
    bool* callback_called = (bool*)user_data;
    *callback_called = true;
}

UTEST(filewatcher_tests, add_watch_modify_0) {
    const char* file_path = "test_file.txt";
    std::ofstream file(file_path);
    file << "File dummy content";
    file.close();

    bool callback_called = false;
    HFileWatcher watcher = FileWatcherCreate(OnFileChanged, &callback_called, 100);
    FileWatcherAddWatch(watcher, file_path);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::ofstream modified_file(file_path);
    modified_file << "Modified content";
    modified_file.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    T(callback_called);

    FileWatcherRemoveWatch(watcher, file_path);
    FileWatcherDestroy(watcher);

    std::remove(file_path);
}

UTEST(filewatcher_tests, add_watch_modify_1) {
    const char* file_path = "test_file.txt";
    std::ofstream file(file_path);
    file << "File dummy content";
    file.close();

    bool callback_called = false;
    HFileWatcher watcher = FileWatcherCreate(OnFileChanged, &callback_called, 16);
    FileWatcherAddWatch(watcher, file_path);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    T(!callback_called);

    FileWatcherRemoveWatch(watcher, file_path);
    FileWatcherDestroy(watcher);

    std::remove(file_path);
}

UTEST(filewatcher_tests, add_watch_modify_2) {
    const char* file_path = "test_file.txt";
    std::ofstream file(file_path);
    file << "File dummy content";
    file.close();

    bool callback_called = false;
    HFileWatcher watcher = FileWatcherCreate(OnFileChanged, &callback_called, 100);
    FileWatcherAddWatch(watcher, file_path);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    FileWatcherRemoveWatch(watcher, file_path);

    std::ofstream modified_file(file_path);
    modified_file << "Modified content";
    modified_file.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    T(!callback_called);

    FileWatcherDestroy(watcher);

    std::remove(file_path);
}

UTEST_MAIN();
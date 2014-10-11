#include <iostream>
#include "shader.h"
#include "GLFW/glfw3.h"
#include <CoreServices/CoreServices.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "filewatcher.h"

typedef struct {
    size_t len;
    size_t size;
    char **paths;
} file_paths_t;

void add_file(file_paths_t *file_paths, char *path);
void event_cb(ConstFSEventStreamRef streamRef,
              void *ctx,
              size_t count,
              void *paths,
              const FSEventStreamEventFlags flags[],
              const FSEventStreamEventId ids[]);

void add_file(file_paths_t *file_paths, char *path) {
    printf("adding %s length %lu size %lu\n", path, file_paths->len, file_paths->size);
    if (file_paths->len == file_paths->size) {
        file_paths->size = file_paths->size * 1.5;
        file_paths->paths = (char**)realloc(file_paths->paths, file_paths->size * sizeof(char *));
    }
    file_paths->paths[file_paths->len] = strdup(path);
    file_paths->len++;
}


void event_cb(ConstFSEventStreamRef streamRef,
              void *ctx,
              size_t count,
              void *paths,
              const FSEventStreamEventFlags flags[],
              const FSEventStreamEventId ids[]) {

    file_paths_t *file_paths = (file_paths_t *)ctx;
    size_t i;
    size_t ignored_paths = 0;

    for (i = 0; i < count; i++) {
        char *path = ((char **)paths)[i];
        /* flags are unsigned long, IDs are uint64_t */
        printf("Change %llu in %s, flags %lu\n", ids[i], path, (long)flags[i]);
        size_t j;
        for (j = 0; j < file_paths->len; j++) {
            char *file_path = file_paths->paths[j];
            printf("%s %s\n", file_path, path);
            if (strcmp(file_path, path) == 0) {
                printf("File %s changed.\n", file_path);
            }
        }
        /* TODO: this logic is wrong */
        ignored_paths++;
    }
    if (count > ignored_paths) {
        /* OS X occasionally leaks event streams. Manually stop the stream just to make sure. */
        FSEventStreamStop((FSEventStreamRef)streamRef);
        exit(0);
    }
}


int main(int argc, char **argv) {
    char* s = new char[1024];
    realpath(argv[1], s);
    string absolutePath(s);
    FileWatcher watcher(absolutePath, NULL);
    cout << "start watching " << absolutePath << endl;
    watcher.startWatching();
}

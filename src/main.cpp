#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "liveglsl.h"

static LiveGLSL* live_glsl;

#ifdef EMSCRIPTEN
#ifndef SHADER_INPUT
#error "SHADER_INPUT not defined"
#endif

extern "C" {

void LiveGLSLSetShader(const char* path) {
    if (live_glsl) {
        LiveGLSLDestroy(live_glsl);
        Arguments args;
        args.Input = path;
        live_glsl = LiveGLSLCreate(args);
    }
}

void LiveGLSLSetCanvasId(const char* id) {
    if (live_glsl) {
        live_glsl->CanvasId = id;
    }
}

};

#endif

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::vector<std::string> args_parsed;
    std::string arg;

    args_parsed.push_back("live-glsl.exe");
    for (const char* p = lpCmdLine; *p != '\0'; ++p) {
        if (*p == ' ' || *p == '\t') {
            if (!arg.empty()) {
                args_parsed.push_back(arg);
                arg.clear();
            }
        } else {
            arg += *p;
        }
    }

    if (!arg.empty()) {
        args_parsed.push_back(arg);
    }

    int argc = args_parsed.size();
    const char** argv = new const char*[argc];
    for (size_t i = 0; i < argc; ++i) {
        argv[i] = args_parsed[i].c_str();
    }
#else
int main(int argc, const char **argv) {
#endif
    Arguments args;
#ifdef EMSCRIPTEN
    args.Input = SHADER_INPUT;
#else
    if (!ArgumentsParse(argc, argv, args)) {
        return EXIT_FAILURE;
    }
#endif

    live_glsl = LiveGLSLCreate(args);
    if (!live_glsl) {
        return EXIT_FAILURE;
    }

    bool res = LiveGLSLRender(live_glsl);
    LiveGLSLDestroy(live_glsl);

#ifdef _WIN32
    delete[] argv;
#endif

    return res;
}

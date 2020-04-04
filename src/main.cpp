#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>

#include "GLFW/glfw3.h"
#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"
#define FONT_SIZE 32

struct ScreenLog {
    FONScontext* FontContext;
    bool LogBuffered;
    std::vector<fsuint> TextHandles;
    std::string Log;
};

struct ShaderProgram {
    GLuint Handle;
    GLuint FragmentShaderHandle;
    GLuint VertexShaderHandle;

    ShaderProgram() {
        std::memset(this, 0x0, sizeof(ShaderProgram));
    }
};

struct LiveGLSL {
    GLFWwindow* GLFWWindowHandle;
    GLuint VertexBufferId;
    GLint PositionVertexAttribute;
    std::string ShaderPath;
    bool ShaderCompiled;
    bool IsContinuousRendering;
    int WindowWidth;
    int WindowHeight;
    float PixelDensity;
};

static std::atomic<bool> ShaderFileChanged;
static std::atomic<bool> ShouldQuit;
static ScreenLog ScreenLogInstance;
struct LiveGLSL;
static LiveGLSL* LiveGLSLInstance;
static ShaderProgram ShaderProgramInstance;
static const GLchar* DefaultVertexShader = R"END(
attribute vec4 position;
void main() {
    gl_Position = position;
}
)END";

std::vector<std::string> SplitString(const std::string& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

bool ReadFile(const std::string& path, std::string& out) {
    std::ifstream file;
    std::string buffer;
    file.open(path.c_str());

    if (file.is_open()) {
        while (!file.eof()) {
            getline(file, buffer);
            out += buffer + "\n";
        }
        file.close();
        return true;
    }

    return false;
}

ScreenLog ScreenLogCreate(float pixel_density) {
    ScreenLog screen_log;
    screen_log.LogBuffered = false;
    screen_log.FontContext = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
    int font = fonsAddFont(screen_log.FontContext, "sans", "/Library/Fonts/Arial.ttf");
    if (font == FONS_INVALID) {
        font = fonsAddFont(screen_log.FontContext, "sans", "/Library/Fonts/Arial Unicode.ttf"); 
    }
    assert(font != FONS_INVALID &&  "Could not load font Arial");
    fonsSetSize(screen_log.FontContext, FONT_SIZE * pixel_density);
    fonsSetFont(screen_log.FontContext, font);
    glfonsUpdateViewport(screen_log.FontContext);
    return screen_log;
}

void ScreenLogDestroy(ScreenLog& screen_log) {
    for(auto id : screen_log.TextHandles) {
        glfonsUnbufferText(screen_log.FontContext, id);
    }
    glfonsDelete(screen_log.FontContext);
}

void ScreenLogRender(ScreenLog& screen_log, float pixel_density) {
    if (screen_log.Log.empty()) {
        if (screen_log.LogBuffered) {
            for(auto id : screen_log.TextHandles) {
                glfonsUnbufferText(screen_log.FontContext, id);
            }

            screen_log.TextHandles.clear();
            screen_log.LogBuffered = false;
        }
    } else {
        if (!screen_log.LogBuffered) {
            fsuint id;
            for(auto str : SplitString(screen_log.Log, '\n')) {
                glfonsBufferText(screen_log.FontContext, str.c_str(), &id, FONS_EFFECT_NONE);
                screen_log.TextHandles.push_back(id);
            }
            screen_log.LogBuffered = true;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfonsUpdateViewport(screen_log.FontContext);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        float y_offset = FONT_SIZE * pixel_density + FONT_SIZE * pixel_density * 0.25f;
        glfonsPushMatrix(screen_log.FontContext);
        glfonsTranslate(screen_log.FontContext, FONT_SIZE * pixel_density * 0.5f, 0.0f);

        for (auto id : screen_log.TextHandles) {
            glfonsTranslate(screen_log.FontContext, 0.0f, y_offset);
            glfonsDrawText(screen_log.FontContext, id);
        }

        glfonsPopMatrix(screen_log.FontContext);
        glDisable(GL_BLEND);
    }
}

void ScreenLogRenderFrameStatus(ScreenLog& screen_log, bool sixty_fps, float pixel_density) {
    char buffer[32];
    sprintf(buffer, "■");
    static fsuint id = 0;
    if (id != 0) { glfonsUnbufferText(screen_log.FontContext, id); }
    glfonsBufferText(screen_log.FontContext, buffer, &id, FONS_EFFECT_NONE);
    glfonsUpdateViewport(screen_log.FontContext);
    if (sixty_fps)
        glfonsSetColor(screen_log.FontContext, 0.21, 1.0, 0.74, 1.0);
    else
        glfonsSetColor(screen_log.FontContext, 1.0, 0.0, 0.0, 1.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glfonsPushMatrix(screen_log.FontContext);
    glfonsTranslate(screen_log.FontContext, FONT_SIZE * pixel_density * 0.5f, FONT_SIZE * pixel_density);
    glfonsDrawText(screen_log.FontContext, id);
    glfonsPopMatrix(screen_log.FontContext);
    glDisable(GL_BLEND);
    glfonsSetColor(screen_log.FontContext, 1.0, 1.0, 1.0, 1.0);
}

void ScreenLogClear(ScreenLog& screen_log) {
    screen_log.Log = "";
    for(auto id : screen_log.TextHandles) {
        glfonsUnbufferText(screen_log.FontContext, id);
    }
    screen_log.TextHandles.clear();
    screen_log.LogBuffered = false;
}

void ScreenLogBuffer(ScreenLog& screen_log, const char* c) {
    screen_log.Log += std::string(c);
}

void ShaderProgramDestroy(ShaderProgram& shader_program) {
    if (shader_program.Handle)
        glDeleteProgram(shader_program.Handle);
    if (shader_program.VertexShaderHandle)
        glDeleteShader(shader_program.VertexShaderHandle);
    if (shader_program.FragmentShaderHandle)
        glDeleteShader(shader_program.FragmentShaderHandle);
}

GLuint CompileShader(const std::string& src, GLenum type) {
    GLint compile_status;
    GLuint shader = glCreateShader(type);
    const GLchar* source = (const GLchar*) src.c_str();

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if (!compile_status) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char info[8096];
        glGetShaderInfoLog(shader, length, NULL, info);
        std::cerr << info << std::endl;
        ScreenLogBuffer(ScreenLogInstance, info);
        glDeleteShader(shader);
        return 0;
    }

    ScreenLogClear(ScreenLogInstance);

    return shader;
}

bool ShaderProgramBuild(ShaderProgram& shader_program, const std::string& fragment_source, const std::string& vertex_source) {
    shader_program.VertexShaderHandle = CompileShader(vertex_source, GL_VERTEX_SHADER);
    shader_program.FragmentShaderHandle = CompileShader(fragment_source, GL_FRAGMENT_SHADER);
    if (!shader_program.FragmentShaderHandle || !shader_program.VertexShaderHandle) {
        ShaderProgramDestroy(shader_program);
        return false;
    }
    shader_program.Handle = glCreateProgram();
    glAttachShader(shader_program.Handle, shader_program.VertexShaderHandle);
    glAttachShader(shader_program.Handle, shader_program.FragmentShaderHandle);
    glLinkProgram(shader_program.Handle);
    glDeleteShader(shader_program.FragmentShaderHandle);
    glDeleteShader(shader_program.VertexShaderHandle);
    GLint is_linked;
    glGetProgramiv(shader_program.Handle, GL_LINK_STATUS, &is_linked);
    if (is_linked == GL_FALSE) {
        ScreenLogBuffer(ScreenLogInstance, "Error linking program");
        ShaderProgramDestroy(shader_program);
        return false;
    }
    ScreenLogClear(ScreenLogInstance);
    return true;
}

void ShaderProgramDetach(const ShaderProgram& shader_program) {
    if (shader_program.VertexShaderHandle)
        glDetachShader(shader_program.VertexShaderHandle, GL_VERTEX_SHADER);
    if (shader_program.FragmentShaderHandle)
        glDetachShader(shader_program.FragmentShaderHandle, GL_FRAGMENT_SHADER);
}

LiveGLSL* LiveGLSLCreate(std::string shader_path) {
    LiveGLSL* live_glsl = new LiveGLSL();
    live_glsl->ShaderCompiled = false;
    live_glsl->ShaderPath = shader_path;
    live_glsl->WindowWidth = 800;
    live_glsl->WindowHeight = 600;
    live_glsl->IsContinuousRendering = false;

    // Init GLFW Window
    {
        glfwInit();
        live_glsl->GLFWWindowHandle = glfwCreateWindow(live_glsl->WindowWidth, live_glsl->WindowHeight, "live-glsl", NULL, NULL);
        glfwMakeContextCurrent(live_glsl->GLFWWindowHandle);
        glfwSetWindowUserPointer(live_glsl->GLFWWindowHandle, live_glsl);
        glfwSetWindowSizeCallback(live_glsl->GLFWWindowHandle, [](GLFWwindow* window, int width, int height) {
            LiveGLSL* live_glsl = static_cast<LiveGLSL*>(glfwGetWindowUserPointer(window));
            live_glsl->WindowWidth = width;
            live_glsl->WindowHeight = height;
        });
        glfwSetFramebufferSizeCallback(live_glsl->GLFWWindowHandle, [](GLFWwindow* window, int fb_width, int fb_height) {
            LiveGLSL* live_glsl = static_cast<LiveGLSL*>(glfwGetWindowUserPointer(window));
            int window_width = 0;
            int window_height = 0;
            glfwGetWindowSize(live_glsl->GLFWWindowHandle, &window_width, &window_height);
            live_glsl->PixelDensity = (float)fb_width / (float)window_width;
        });
        glfwSetKeyCallback(live_glsl->GLFWWindowHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GL_TRUE);
        });
        int fb_width = 0;
        int fb_height = 0;
        glfwGetFramebufferSize(live_glsl->GLFWWindowHandle, &fb_width, &fb_height);
        live_glsl->PixelDensity = (float)fb_width / (float)live_glsl->WindowWidth;
    }

    ScreenLogInstance = ScreenLogCreate(live_glsl->PixelDensity);

    // Compile shader
    {
        std::string shader_source;
        if (!ReadFile(shader_path, shader_source)) {
            ScreenLogBuffer(ScreenLogInstance, std::string("Unable read file at path " + shader_path).c_str());
        }

        live_glsl->ShaderCompiled = ShaderProgramBuild(ShaderProgramInstance, shader_source, DefaultVertexShader);
    }

    // Init GL
    {
        glClearColor(0.21, 0.39, 0.74, 1.0);
        float vertices[12] = {
            -1.0f,  1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f,
            -1.0f, -1.0f,
             1.0f, -1.0f,
        };

        glGenBuffers(1, &live_glsl->VertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, live_glsl->VertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        live_glsl->PositionVertexAttribute = glGetAttribLocation(ShaderProgramInstance.Handle, "position");
        glVertexAttribPointer(live_glsl->PositionVertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(live_glsl->PositionVertexAttribute);
    }

    return live_glsl;
}

void LiveGLSLDestroy(LiveGLSL* live_glsl) {
    if (live_glsl->VertexBufferId) 
        glDeleteBuffers(1, &live_glsl->VertexBufferId);
    delete live_glsl;
}

void LiveGLSLRender(LiveGLSL& live_glsl) {
    double previous_time = glfwGetTime();
    uint32_t frame_count = 0;
    bool sixty_fps = false;

    while (!glfwWindowShouldClose(live_glsl.GLFWWindowHandle)) {
        std::string shader_source;
        if (ShaderFileChanged && ReadFile(live_glsl.ShaderPath, shader_source)) {
            ShaderProgramDetach(ShaderProgramInstance);
            live_glsl.ShaderCompiled = ShaderProgramBuild(ShaderProgramInstance, shader_source, DefaultVertexShader);
            ShaderFileChanged.store(false);
            glfwPostEmptyEvent();
        }

        double current_time = glfwGetTime();
        ++frame_count;
        if (current_time - previous_time >= 1.0)
        {
            sixty_fps = frame_count >= 60;
            frame_count = 0;
            previous_time = current_time;
        }

        if (live_glsl.ShaderCompiled) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            assert(ShaderProgramInstance.Handle != 0);
            glUseProgram(ShaderProgramInstance.Handle);

            glBindBuffer(GL_ARRAY_BUFFER, live_glsl.VertexBufferId);
            float width = live_glsl.WindowWidth * live_glsl.PixelDensity;
            float height = live_glsl.WindowHeight * live_glsl.PixelDensity;
            glViewport(0, 0, width, height);
            glUniform2f(glGetUniformLocation(ShaderProgramInstance.Handle, "resolution"), width, height);
            glUniform1f(glGetUniformLocation(ShaderProgramInstance.Handle, "time"), glfwGetTime());
            glVertexAttribPointer(live_glsl.PositionVertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(live_glsl.PositionVertexAttribute);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            live_glsl.IsContinuousRendering = glGetUniformLocation(ShaderProgramInstance.Handle, "time") != -1;

            if (live_glsl.IsContinuousRendering)
                ScreenLogRenderFrameStatus(ScreenLogInstance, sixty_fps, live_glsl.PixelDensity);
        }

        ScreenLogRender(ScreenLogInstance, live_glsl.PixelDensity);

        glfwSwapBuffers(live_glsl.GLFWWindowHandle);
        if (live_glsl.IsContinuousRendering)
            glfwPollEvents();
        else
            glfwWaitEvents();
    }

    glfwTerminate();
}

void FileWatcherThread(std::string shader_source_path) {
    char real_path[8096];
    realpath(shader_source_path.c_str(), real_path);
    int last_changed;
    struct stat st;
    stat(real_path, &st);
    while (!ShouldQuit) {
        stat(real_path, &st);
        if(st.st_mtime != last_changed) {
            ShaderFileChanged.store(true);
            last_changed = st.st_mtime;
            glfwPostEmptyEvent();
        }
        usleep(16000);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Please provide the fragment shader source path" << std::endl;
        return -1;
    }

    ShaderFileChanged.store(false);
    ShouldQuit.store(false);
    std::string shader_source_path = std::string(argv[1]);
    LiveGLSLInstance = LiveGLSLCreate(shader_source_path);
    if (!LiveGLSLInstance)
        return 0;

    std::thread file_watcher_thread(FileWatcherThread, shader_source_path);
    LiveGLSLRender(*LiveGLSLInstance);
    ShouldQuit.store(true);
    file_watcher_thread.join();

    LiveGLSLDestroy(LiveGLSLInstance);

    return 0;
}

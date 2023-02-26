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

#include <getopt/getopt.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFONTSTASH_IMPLEMENTATION
#include <glfontstash.h>
#define FONT_SIZE 32

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "gui.h"
#include "arial.ttf.h"
#include "defines.h"

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

struct Arguments {
    std::string Input;
    std::string Output;
    uint32_t Width = 800;
    uint32_t Height = 600;
};

struct LiveGLSL {
    std::vector<GUIComponent> GUIComponents;
    GLFWwindow* GLFWWindowHandle;
    Arguments Args;
    GLuint VertexBufferId;
    GLuint VaoId;
    GLint PositionVertexAttribute;
    std::string ShaderPath;
    bool ShaderCompiled;
    bool IsContinuousRendering;
    int WindowWidth;
    int WindowHeight;
    float PixelDensity;
};

enum Option {
    OPTION_INPUT = 1,
    OPTION_OUTPUT,
    OPTION_WIDTH,
    OPTION_HEIGHT
};

static const getopt_option_t option_list[] = {
    { "input",      'i', GETOPT_OPTION_TYPE_REQUIRED,   0, OPTION_INPUT,        "input source file", "GLSL file" },
    { "output",     'o', GETOPT_OPTION_TYPE_REQUIRED,   0, OPTION_OUTPUT,       "output source file", "PNG file" },
    { "width",      'w', GETOPT_OPTION_TYPE_REQUIRED,   0, OPTION_WIDTH,        "viewport width, in pixels (default 800)" },
    { "height",     'h', GETOPT_OPTION_TYPE_REQUIRED,   0, OPTION_HEIGHT,       "viewport height, in pixels (default 600)" },
    GETOPT_OPTIONS_END
};

static std::atomic<bool> ShaderFileChanged;
static std::atomic<bool> ShouldQuit;
static ScreenLog ScreenLogInstance;
struct LiveGLSL;
static LiveGLSL* LiveGLSLInstance;
static ShaderProgram ShaderProgramInstance;
static const GLchar* DefaultVertexShader = R"END(
in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
)END";

std::vector<std::string> SplitString(const std::string& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty() && item != ";") {
            elems.push_back(item);
        }
    }
    return elems;
}

bool ParseGUIComponent(uint32_t line_number, const std::string& gui_component_line, const std::string& uniform_line, const std::vector<GUIComponent>& previous_components, GUIComponent& out_component, std::string& out_parse_error) {
    auto ReportError = [&](const std::string& error) {
        char buffer[33];
        sprintf(buffer, "%d", line_number);
        out_parse_error = error + " at line " + buffer;
    };
    if (uniform_line.empty()) {
        ReportError("GUI type not associated with any uniform '" + gui_component_line + "'");
        return false;
    }
    uint32_t current_char = 0;
    while (current_char < gui_component_line.size() && gui_component_line[current_char] != '(') {
        ++current_char;
    }

    std::string component_name = gui_component_line.substr(0, current_char);
    std::string component_data = gui_component_line.substr(current_char, std::string::npos);
    if (component_name == "slider1") {
        out_component.Type = EGUIComponentTypeSlider1;
    } else if (component_name == "slider2") {
        out_component.Type = EGUIComponentTypeSlider2;
    } else if (component_name == "slider3") {
        out_component.Type = EGUIComponentTypeSlider3;
    } else if (component_name == "slider4") {
        out_component.Type = EGUIComponentTypeSlider4;
    } else if (component_name == "drag1") {
        out_component.Type = EGUIComponentTypeDrag1;
    } else if (component_name == "drag2") {
        out_component.Type = EGUIComponentTypeDrag2;
    } else if (component_name == "drag3") {
        out_component.Type = EGUIComponentTypeDrag3;
    } else if (component_name == "drag4") {
        out_component.Type = EGUIComponentTypeDrag4;
    } else if (component_name == "color3") {
        out_component.Type = EGUIComponentTypeColor3;
    } else if (component_name == "color4") {
        out_component.Type = EGUIComponentTypeColor4;
    } else {
        ReportError("Invalid GUI type name '" + component_name + "'");
        return false;
    }

    if (!component_data.empty()) {
        if (GUIIsSliderType(out_component.Type)) {
            int scanned = sscanf(component_data.c_str(), "(%f, %f)",
                &out_component.SliderRange.Start,
                &out_component.SliderRange.End);
            if (scanned != 2) {
                std::string error;
                error += "Invalid format for GUI component data '" + component_data + "'\n";
                error += "Format should be @" + component_name + "(start_range, end_range)";
                ReportError(error);
                return false;
            }
        } else if (GUIIsDragType(out_component.Type)) {
            int scanned = sscanf(component_data.c_str(), "(%f, %f, %f)",
                &out_component.DragRange.Speed,
                &out_component.DragRange.Start,
                &out_component.DragRange.End);
            if (scanned != 3) {
                std::string error;
                error += "Invalid format for GUI component data " + component_data + "\n";
                error += "Format should be @" + component_name + "(speed, start_range, end_range)";
                ReportError(error);
                return false;
            }
        }
    } else {
        // Initialize the data with convenient defaults
        if (GUIIsSliderType(out_component.Type)) {
            out_component.SliderRange.Start = 0.0f;
            out_component.SliderRange.End = 1.0f;
        } else if (GUIIsDragType(out_component.Type)) {
            out_component.DragRange.Speed = 0.05f;
            out_component.DragRange.Start = 0.0f;
            out_component.DragRange.End = 1.0f;
        }
    }

    std::vector<std::string> uniform_tokens = SplitString(uniform_line,  ' ');
    if (uniform_tokens.size() != 3) {
        ReportError("GUI associated with invalid uniform");
        return false;
    }
    if (uniform_tokens[0] != "uniform") {
        ReportError("GUI type not associated with a uniform");
        return false;
    }
    std::string uniform_type = uniform_tokens[1];
    if (uniform_type == "float") {
        out_component.UniformType = EGUIUniformTypeFloat;
    } else if (uniform_type == "vec2") {
        out_component.UniformType = EGUIUniformTypeVec2;
    } else if (uniform_type == "vec3") {
        out_component.UniformType = EGUIUniformTypeVec3;
    } else {
        ReportError("GUI not supported for uniform type '" + uniform_type + "'");
        return false;
    }

    if (GUIComponents(out_component.Type) != GUIUniformVariableComponents(out_component.UniformType)) {
        ReportError("GUI component count does not match uniform component count");
        return false;
    }

    // Trim semicolon ';'
    while (uniform_tokens.back().back() == ';')
        uniform_tokens.back().pop_back();
    out_component.UniformName = uniform_tokens.back();

    if (out_component.UniformName == "time" || out_component.UniformName == "resolution") {
        ReportError("GUI type not allowed for builtin uniforms");
        return false;
    }

    for (const GUIComponent& previous_component : previous_components) {
        if (previous_component.UniformName == out_component.UniformName) {
            std::memcpy(&out_component.Vec1, &previous_component.Vec1, sizeof(glm::vec4));
        }
    }
    return true;
}

bool ReadShaderFile(const std::string& path, std::string& out, std::vector<GUIComponent>& out_components, std::string& read_file_error) {
    std::ifstream file;
    std::string curr_buffer;
    std::string prev_buffer;
    file.open(path.c_str());

    if (!file.is_open()) {
        read_file_error = "Unable read file at path " + path;
        return false;
    }

    std::vector<GUIComponent> previous_components = out_components;
    out_components.clear();
    uint32_t line_number = 0;
    while (!file.eof()) {
        getline(file, curr_buffer);
        uint32_t current_char = 0;
        while (isspace(curr_buffer[current_char])) {
            ++current_char;
        }
        if (prev_buffer[current_char] == '@') {
            GUIComponent component;
            std::string gui_component_line = prev_buffer.substr(current_char + 1, std::string::npos);
            if (!ParseGUIComponent(line_number, gui_component_line, curr_buffer, previous_components, component, read_file_error))
                return false;
            out_components.push_back(component);
        }
        if (curr_buffer[current_char] != '@')
            out += curr_buffer + "\n";
        prev_buffer = curr_buffer;
        ++line_number;
    }
    file.close();
    return true;
}

ScreenLog ScreenLogCreate(float pixel_density) {
    ScreenLog screen_log;
    screen_log.LogBuffered = false;
    screen_log.FontContext = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
    int font = fonsAddFontMem(screen_log.FontContext, "sans", arial_ttf, ARRAY_LENGTH(arial_ttf), 1);
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
        float y_offset = FONT_SIZE * pixel_density + FONT_SIZE * pixel_density * 0.25f;
        glfonsPushMatrix(screen_log.FontContext);
        glfonsTranslate(screen_log.FontContext, FONT_SIZE * pixel_density * 0.5f, 0.0f);

        for (auto id : screen_log.TextHandles) {
            glfonsTranslate(screen_log.FontContext, 0.0f, y_offset);
            glfonsDrawText(screen_log.FontContext, id);
        }

        glfonsPopMatrix(screen_log.FontContext);
    }
}

void ScreenLogRenderFrameStatus(ScreenLog& screen_log, uint32_t screen_width, bool sixty_fps, float pixel_density) {
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
    glfonsPushMatrix(screen_log.FontContext);
    glfonsTranslate(screen_log.FontContext, screen_width - FONT_SIZE * pixel_density, FONT_SIZE * pixel_density);
    glfonsDrawText(screen_log.FontContext, id);
    glfonsPopMatrix(screen_log.FontContext);
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

GLuint CompileShader(const std::string src, GLenum type) {
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
        fprintf(stderr, "%s\n", src.c_str());
        fprintf(stderr, "%s\n", info);
        ScreenLogBuffer(ScreenLogInstance, info);
        glDeleteShader(shader);
        return 0;
    }

    ScreenLogClear(ScreenLogInstance);

    return shader;
}

bool ShaderProgramBuild(ShaderProgram& shader_program, const std::string& fragment_source, const std::string& vertex_source) {
    std::string shader_prelude = "#version 150\n";
    shader_program.VertexShaderHandle = CompileShader(shader_prelude + vertex_source, GL_VERTEX_SHADER);
    shader_program.FragmentShaderHandle = CompileShader(shader_prelude + fragment_source, GL_FRAGMENT_SHADER);
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

LiveGLSL* LiveGLSLCreate(const Arguments& args) {
    LiveGLSL* live_glsl = new LiveGLSL();
    live_glsl->ShaderCompiled = false;
    live_glsl->ShaderPath = args.Input;
    live_glsl->WindowWidth = args.Width;
    live_glsl->WindowHeight = args.Height;
    live_glsl->IsContinuousRendering = false;
    live_glsl->Args = args;

    // Init GLFW Window
    {
        if (!glfwInit()) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        live_glsl->GLFWWindowHandle = glfwCreateWindow(live_glsl->WindowWidth, live_glsl->WindowHeight, "live-glsl", NULL, NULL);
        if (!live_glsl->GLFWWindowHandle) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(live_glsl->GLFWWindowHandle);
        glfwSetWindowUserPointer(live_glsl->GLFWWindowHandle, live_glsl);
        glfwSetErrorCallback([](int error, const char* description) {
            fprintf(stderr, "Error: %s\n", description);
        });
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
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        glfwSwapInterval(1);
    }

    GUIInit(live_glsl->GLFWWindowHandle);

    ScreenLogInstance = ScreenLogCreate(live_glsl->PixelDensity);

    // Compile shader
    {
        std::string shader_source;
        std::string read_file_error;
        if (!ReadShaderFile(args.Input, shader_source, live_glsl->GUIComponents, read_file_error)) {
            ScreenLogBuffer(ScreenLogInstance, read_file_error.c_str());
        } else {
            live_glsl->ShaderCompiled = ShaderProgramBuild(ShaderProgramInstance, shader_source, DefaultVertexShader);
        }
    }

    // Init GL
    {
        glClearColor(0.21, 0.39, 0.74, 1.0);
        const float vertices[] = {
            -1.0f,  1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f,
            -1.0f, -1.0f,
             1.0f, -1.0f,
        };

        glGenVertexArrays(1, &live_glsl->VaoId);
        glBindVertexArray(live_glsl->VaoId);
        glGenBuffers(1, &live_glsl->VertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, live_glsl->VertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        live_glsl->PositionVertexAttribute = glGetAttribLocation(ShaderProgramInstance.Handle, "position");
        glVertexAttribPointer(live_glsl->PositionVertexAttribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        glEnableVertexAttribArray(live_glsl->PositionVertexAttribute);
    }

    return live_glsl;
}

void LiveGLSLDestroy(LiveGLSL* live_glsl) {
    if (live_glsl->VertexBufferId)
        glDeleteBuffers(1, &live_glsl->VertexBufferId);
    if (live_glsl->VaoId)
        glDeleteVertexArrays(1, &live_glsl->VaoId);
    delete live_glsl;
}

int LiveGLSLRender(LiveGLSL& live_glsl) {
    double previous_time = glfwGetTime();
    uint32_t frame_count = 0;
    bool sixty_fps = false;

    while (!glfwWindowShouldClose(live_glsl.GLFWWindowHandle)) {
        std::string shader_source;
        std::string read_file_error;
        if (ShaderFileChanged && ReadShaderFile(live_glsl.ShaderPath, shader_source, live_glsl.GUIComponents, read_file_error)) {
            ShaderProgramDetach(ShaderProgramInstance);
            live_glsl.ShaderCompiled = ShaderProgramBuild(ShaderProgramInstance, shader_source, DefaultVertexShader);
            glfwPostEmptyEvent();
        }
        ShaderFileChanged.store(false);

        if (!read_file_error.empty()) {
            ScreenLogBuffer(ScreenLogInstance, read_file_error.c_str());
        }

        double current_time = glfwGetTime();
        ++frame_count;
        if (current_time - previous_time >= 1.0) {
            sixty_fps = frame_count >= 60;
            frame_count = 0;
            previous_time = current_time;
        }
        uint32_t width = live_glsl.WindowWidth * live_glsl.PixelDensity;
        uint32_t height = live_glsl.WindowHeight * live_glsl.PixelDensity;

        if (live_glsl.ShaderCompiled) {
            for (GUIComponent& gui_component : live_glsl.GUIComponents) {
                GLuint uniform_location = glGetUniformLocation(ShaderProgramInstance.Handle, gui_component.UniformName.c_str());
                gui_component.IsInUse = uniform_location != -1;
            }

            bool draw_gui = GUINewFrame(live_glsl.GUIComponents);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            assert(ShaderProgramInstance.Handle != 0);
            glUseProgram(ShaderProgramInstance.Handle);

            double x, y;
            glfwGetCursorPos(live_glsl.GLFWWindowHandle, &x, &y);
            x *= live_glsl.PixelDensity;
            y *= live_glsl.PixelDensity;

            int mouse_left_state = glfwGetMouseButton(live_glsl.GLFWWindowHandle, GLFW_MOUSE_BUTTON_LEFT);

            glBindBuffer(GL_ARRAY_BUFFER, live_glsl.VertexBufferId);
            glViewport(0, 0, width, height);
            glUniform2f(glGetUniformLocation(ShaderProgramInstance.Handle, "resolution"), width, height);
            glUniform1f(glGetUniformLocation(ShaderProgramInstance.Handle, "time"), glfwGetTime());
            // Mouse position, x relative to left, y relative to top
            glUniform3f(glGetUniformLocation(ShaderProgramInstance.Handle, "mouse"), x, y, mouse_left_state == GLFW_PRESS ? 1.0f : 0.0f);

            for (const GUIComponent& gui_component : live_glsl.GUIComponents) {
                GLuint uniform_location = glGetUniformLocation(ShaderProgramInstance.Handle, gui_component.UniformName.c_str());
                switch (gui_component.UniformType) {
                    case EGUIUniformTypeFloat:
                    glUniform1f(uniform_location, gui_component.Vec1);
                    break;
                    case EGUIUniformTypeVec2:
                    glUniform2f(uniform_location, gui_component.Vec2.x, gui_component.Vec2.y);
                    break;
                    case EGUIUniformTypeVec3:
                    glUniform3f(uniform_location, gui_component.Vec3.x, gui_component.Vec3.y, gui_component.Vec3.z);
                    break;
                    case EGUIUniformTypeVec4:
                    glUniform4f(uniform_location, gui_component.Vec4.x, gui_component.Vec4.y, gui_component.Vec4.z, gui_component.Vec4.w);
                    break;
                }
            }

            glBindVertexArray(live_glsl.VaoId);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            live_glsl.IsContinuousRendering = glGetUniformLocation(ShaderProgramInstance.Handle, "time") != -1;

            if (live_glsl.Args.Output.empty()) {
                if (live_glsl.IsContinuousRendering) {
                    ScreenLogRenderFrameStatus(ScreenLogInstance, width, sixty_fps, live_glsl.PixelDensity);
                }

                if (draw_gui) {
                    GUIRender();
                }
            }
        }

        if (!live_glsl.Args.Output.empty()) {
            unsigned char* pixels = new unsigned char[3 * width * height];
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

            int res = stbi_write_png(live_glsl.Args.Output.c_str(), width, height, 3, pixels, 3 * width);
            delete[] pixels;
            if (res == 0) {
                printf("Failed to write image to file: %s\n", live_glsl.Args.Output.c_str());
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        } else {
            ScreenLogRender(ScreenLogInstance, live_glsl.PixelDensity);    
        }

        glfwSwapBuffers(live_glsl.GLFWWindowHandle);
        if (live_glsl.IsContinuousRendering)
            glfwPollEvents();
        else
            glfwWaitEvents();
    }

    return EXIT_SUCCESS;
}

void FileWatcherThread(std::string shader_source_path) {
    char real_path[8096];
    realpath(shader_source_path.c_str(), real_path);
    int last_changed;
    struct stat st;
    stat(real_path, &st);
    while (!ShouldQuit) {
        stat(real_path, &st);
        if (st.st_mtime != last_changed) {
            ShaderFileChanged.store(true);
            last_changed = st.st_mtime;
            glfwPostEmptyEvent();
        }
        usleep(16000);
    }
}

bool ParseArguments(int argc, const char** argv, Arguments& args) {
    getopt_context_t ctx;
    if (getopt_create_context(&ctx, argc, argv, option_list) < 0) {
        printf( "error while creating getopt ctx, bad options-list?" );
        return false;
    }
    int opt = 0;
    while ((opt = getopt_next(&ctx)) != -1) {
        switch (opt) {
            case '+':
                printf("live-glsl: got argument without flag: %s\n", ctx.current_opt_arg);
                return false;
            case '?':
                printf("live-glsl: unknown flag %s\n", ctx.current_opt_arg);
                return false;
            case '!':
                printf("live-glsl: invalid use of flag %s\n", ctx.current_opt_arg);
                return false;
            case OPTION_INPUT:
                args.Input = ctx.current_opt_arg;
                break;
            case OPTION_OUTPUT:
                args.Output = ctx.current_opt_arg;
                break;
            case OPTION_WIDTH:
                args.Width = atoi(ctx.current_opt_arg);
                break;
            case OPTION_HEIGHT:
                args.Height = atoi(ctx.current_opt_arg);
                break;
            default:
                break;
        }
    }
    if (args.Input.empty()) {
        printf("live-glsl: no input file (--input [path])\n");
        return false;
    }
    return true;
}

int main(int argc, const char **argv) {
    Arguments args;
    if (!ParseArguments(argc, argv, args)) {
        return EXIT_FAILURE;
    }

    ShaderFileChanged.store(false);
    ShouldQuit.store(false);
    LiveGLSLInstance = LiveGLSLCreate(args);
    if (!LiveGLSLInstance) {
        return EXIT_FAILURE;
    }

    std::thread file_watcher_thread(FileWatcherThread, args.Input);
    bool res = LiveGLSLRender(*LiveGLSLInstance);
    ShouldQuit.store(true);
    file_watcher_thread.join();

    LiveGLSLDestroy(LiveGLSLInstance);

    glfwTerminate();

    return res;
}

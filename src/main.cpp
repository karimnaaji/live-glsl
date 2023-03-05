#include <atomic>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <regex>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFONTSTASH_IMPLEMENTATION
#include <glfontstash.h>
#define FONT_SIZE 20

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "gui.h"
#include "arial.ttf.h"
#include "defines.h"
#include "filewatcher.h"
#include "arguments.h"

#ifdef _WIN32
#define PATH_DELIMITER '\\'
#else
#define PATH_DELIMITER '/'
#endif

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

struct RenderPass {
    ShaderProgram Program;
    std::string ShaderSource;
    std::string Input;
    std::string Output;
    bool IsMain {false};
    uint32_t Width {0};
    uint32_t Height {0};
    GLuint FBO {0};
    GLuint Texture {0};
};

struct LiveGLSL {
    std::vector<GUIComponent> GUIComponents;
    std::vector<RenderPass> RenderPasses;
    GLFWwindow* GLFWWindowHandle;
    Arguments Args;
    GLuint VertexBufferId;
    GLuint VaoId;
    std::string ShaderPath;
    std::string BasePath;
    bool ShaderCompiled;
    bool IsContinuousRendering;
    int WindowWidth;
    int WindowHeight;
    float PixelDensity;
};

static std::atomic<bool> ShaderFileChanged;
static ScreenLog ScreenLogInstance;
struct LiveGLSL;
static LiveGLSL* LiveGLSLInstance;
static HFileWatcher FileWatcher;
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

std::string ExtractBasePath(const std::string& path) {
    const char* cpath = path.c_str();
    const char* last_slash = strrchr(cpath, PATH_DELIMITER);
    std::string base_path;
    if (last_slash != nullptr) {
        size_t parent_path_len = last_slash - cpath;
        base_path = path.substr(0, parent_path_len);
    }
    return base_path;
}

bool Amalgamate(const std::string& base_path, const std::string& path, std::vector<std::string>& watches, std::string& read_file_error, std::string& amalgamate) {
    std::ifstream file;
    std::string curr_buffer;
    std::string prev_buffer;
    file.open(path.c_str());

    if (!file.is_open()) {
        read_file_error = "Unable read file at path " + path;
        return false;
    }

    watches.push_back(path);

    while (!file.eof()) {
        getline(file, curr_buffer);
        uint32_t current_char = 0;
        while (isspace(curr_buffer[current_char])) {
            ++current_char;
        }
        std::regex include_regex("#include\\s+\"([^\"]+)\"");
        std::smatch include_match;
        if (std::regex_search(curr_buffer, include_match, include_regex)) {
            std::string include = base_path + PATH_DELIMITER + std::string(include_match[1]);
            if (!Amalgamate(ExtractBasePath(include), include, watches, read_file_error, amalgamate)) {
                return false;
            }
        } else {
            amalgamate += curr_buffer + '\n';
        }
    }
    file.close();
    return true;
}

bool ReadShaderFile(const std::string& base_path, const std::string& path, std::vector<std::string>& watches, std::vector<RenderPass>& render_passes, std::vector<GUIComponent>& components, std::string& read_file_error) {
    std::string amalgamate;

    if (!Amalgamate(base_path, path, watches, read_file_error, amalgamate)) {
        read_file_error = "Unable read file at path " + path;
        return false;
    }

    std::vector<GUIComponent> previous_components = components;
    components.clear();
    uint32_t line_number = 0;
    RenderPass* pass = nullptr;
    std::string shader_source;
    std::string prev_buffer;
    auto ReportError = [&](const std::string& error, uint32_t line_number) {
        char buffer[33];
        sprintf(buffer, "%d", line_number);
        read_file_error = error + " at line " + buffer;
    };
    for (size_t i = 0; i < amalgamate.size(); ++i) {
        std::string line;
        while (amalgamate[i] != '\n') {
            line += amalgamate[i];
            ++i;
        }
        uint32_t current_char = 0;
        while (isspace(line[current_char])) {
            ++current_char;
        }

        if (current_char < prev_buffer.length() && prev_buffer[current_char] == '@') {
            if (prev_buffer.substr(current_char + 1, current_char + 8) == "pass_end") {
                pass->ShaderSource = shader_source;
                shader_source = "";
                pass = nullptr;
            } else if (prev_buffer.substr(current_char + 1, current_char + 4) == "pass") {
                render_passes.emplace_back();
                pass = &render_passes.back();

                char input_name[64] = {0};
                char output_name[64] = {0};
                uint32_t width = 0;
                uint32_t height = 0;
                std::string render_pass_args = prev_buffer.substr(current_char + 5, std::string::npos);
                int scanned = 0;
                scanned = sscanf(render_pass_args.c_str(), "(%[^,], %[^,], %d, %d)", output_name, input_name, &width, &height);
                if (scanned != 4) {
                    width = height = 0;
                    memset(input_name, 0x0, sizeof(input_name));
                    memset(output_name, 0x0, sizeof(output_name));
                    scanned = sscanf(render_pass_args.c_str(), "(%[^,], %d, %d)", output_name, &width, &height);
                    if (scanned != 3) {
                        width = height = 0;
                        memset(input_name, 0x0, sizeof(input_name));
                        memset(output_name, 0x0, sizeof(output_name));
                        scanned = sscanf(render_pass_args.c_str(), "(%[^,], %[^)])", output_name, input_name);
                        if (scanned != 2) {
                            std::string error;
                            error += "Render pass format should be @pass(output, input, width, height), @pass(output, input) or @pass(output, width, height)";
                            ReportError(error, line_number);
                            return false;
                        }
                    }
                }
                pass->Input = input_name;
                pass->Output = output_name;
                pass->Width = width;
                pass->Height = height;
                if (pass->Output == "main") {
                    pass->IsMain = true;
                }
            } else {
                GUIComponent component;
                std::string gui_component_line = prev_buffer.substr(current_char + 1, std::string::npos);
                if (!ParseGUIComponent(line_number, gui_component_line, line, previous_components, component, read_file_error)) {
                    return false;
                }
                components.push_back(component);
            }
        }
        if (line[current_char] != '@') {
            shader_source += line + "\n";
        }
        prev_buffer = line;
        ++line_number;
    }
    if (render_passes.empty()) {
        render_passes.emplace_back();
        render_passes.back().ShaderSource = shader_source;
        render_passes.back().IsMain = true;
    }
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

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
        fprintf(stderr, "Failed to link program:\n%s", fragment_source.c_str());
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

void DestroyRenderPasses(std::vector<RenderPass>& render_passes) {
    for (auto& render_pass : render_passes) {
        ShaderProgramDetach(render_pass.Program);
        if (render_pass.Texture != 0) {
            glDeleteTextures(1, &render_pass.Texture);
        }
        if (render_pass.FBO != 0) {
            glDeleteFramebuffers(1, &render_pass.FBO);
        }
    }
    render_passes.clear();
}

bool BuildRenderPasses(std::vector<RenderPass>& render_passes) {
    for (auto& render_pass : render_passes) {
        if (!ShaderProgramBuild(render_pass.Program, render_pass.ShaderSource, DefaultVertexShader)) {
            return false;
        }
        if (!render_pass.IsMain) {
            glGenFramebuffers(1, &render_pass.FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, render_pass.FBO);
            glGenTextures(1, &render_pass.Texture);
            glBindTexture(GL_TEXTURE_2D, render_pass.Texture);
            assert(render_pass.Width != 0);
            assert(render_pass.Height != 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, render_pass.Width, render_pass.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_pass.Texture, 0);
            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    return true;
}

void ReloadShaderIfChanged(LiveGLSL& live_glsl, std::string path, bool first_load = false) {
    if (!ShaderFileChanged && !first_load) {
        return;
    }
    std::string read_file_error;
    std::vector<RenderPass> render_passes;
    std::vector<std::string> watches;
    if (!ReadShaderFile(live_glsl.BasePath, path, watches, render_passes, live_glsl.GUIComponents, read_file_error)) {
        if (!read_file_error.empty()) {
            ScreenLogBuffer(ScreenLogInstance, read_file_error.c_str());
        }
    } else {
        live_glsl.ShaderCompiled = BuildRenderPasses(render_passes);
        live_glsl.RenderPasses = render_passes;
        FileWatcherRemoveAllWatches(FileWatcher);
        for (const auto& watch : watches) {
            FileWatcherAddWatch(FileWatcher, watch.c_str());
        }
        glfwPostEmptyEvent();
    }
    ShaderFileChanged.store(false); 
}

LiveGLSL* LiveGLSLCreate(const Arguments& args) {
    LiveGLSL* live_glsl = new LiveGLSL();
    live_glsl->ShaderCompiled = false;
    live_glsl->ShaderPath = args.Input;
    live_glsl->WindowWidth = args.Width;
    live_glsl->WindowHeight = args.Height;
    live_glsl->IsContinuousRendering = false;
    live_glsl->Args = args;
    live_glsl->BasePath = ExtractBasePath(args.Input);

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
        glfwSetDropCallback(live_glsl->GLFWWindowHandle, [](GLFWwindow* window, int count, const char** paths) {
            if (count >= 0) {
                LiveGLSL* live_glsl = static_cast<LiveGLSL*>(glfwGetWindowUserPointer(window));
                std::string path = std::string(paths[0]);
                live_glsl->ShaderPath = path;
                live_glsl->Args.Input = path;
                live_glsl->BasePath = ExtractBasePath(path);
            
                ReloadShaderIfChanged(*live_glsl, path, true);
            }
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

    ReloadShaderIfChanged(*live_glsl, args.Input, true);

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
        ReloadShaderIfChanged(live_glsl, live_glsl.ShaderPath);

        double current_time = glfwGetTime();
        ++frame_count;
        if (current_time - previous_time >= 1.0) {
            sixty_fps = frame_count >= 60;
            frame_count = 0;
            previous_time = current_time;
        }

        double x, y;
        glfwGetCursorPos(live_glsl.GLFWWindowHandle, &x, &y);
        x *= live_glsl.PixelDensity;
        y *= live_glsl.PixelDensity;

        int mouse_left_state = glfwGetMouseButton(live_glsl.GLFWWindowHandle, GLFW_MOUSE_BUTTON_LEFT);

        if (live_glsl.ShaderCompiled) {
            for (GUIComponent& gui_component : live_glsl.GUIComponents) {
                gui_component.IsInUse = false;
                for (const auto& render_pass : live_glsl.RenderPasses) {
                    GLuint uniform_location = glGetUniformLocation(render_pass.Program.Handle, gui_component.UniformName.c_str());
                    gui_component.IsInUse |= uniform_location != -1;
                }
            }

            bool draw_gui = GUINewFrame(live_glsl.GUIComponents);

            for (const auto& render_pass : live_glsl.RenderPasses) {
                uint32_t width = render_pass.IsMain ? live_glsl.WindowWidth : render_pass.Width;
                uint32_t height = render_pass.IsMain ? live_glsl.WindowHeight : render_pass.Height;

                width *= live_glsl.PixelDensity;
                height *= live_glsl.PixelDensity;

                glBindFramebuffer(GL_FRAMEBUFFER, render_pass.FBO);
                glClear(GL_COLOR_BUFFER_BIT);

                assert(render_pass.Program.Handle != 0);
                glUseProgram(render_pass.Program.Handle);

                glBindBuffer(GL_ARRAY_BUFFER, live_glsl.VertexBufferId);
                glViewport(0, 0, width, height);
                glUniform2f(glGetUniformLocation(render_pass.Program.Handle, "resolution"), width, height);
                glUniform1f(glGetUniformLocation(render_pass.Program.Handle, "time"), glfwGetTime());
                glUniform1f(glGetUniformLocation(render_pass.Program.Handle, "pixel_ratio"), live_glsl.PixelDensity);
                // Mouse position, x relative to left, y relative to top
                glUniform3f(glGetUniformLocation(render_pass.Program.Handle, "mouse"), x, y, mouse_left_state == GLFW_PRESS ? 1.0f : 0.0f);

                if (!render_pass.Input.empty()) {
                    for (const auto& other : live_glsl.RenderPasses) {
                        if (other.Output == render_pass.Input) {
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, other.Texture);
                            glUniform1i(glGetUniformLocation(render_pass.Program.Handle, render_pass.Input.c_str()), 0);
                        }
                    }
                }

                for (const GUIComponent& gui_component : live_glsl.GUIComponents) {
                    GLuint uniform_location = glGetUniformLocation(render_pass.Program.Handle, gui_component.UniformName.c_str());
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
                for (auto& render_pass : live_glsl.RenderPasses) {
                    if (render_pass.IsMain) {
                        GLint position_attrib = glGetAttribLocation(render_pass.Program.Handle, "position");
                        glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
                        glEnableVertexAttribArray(position_attrib);
                    }
                }
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            live_glsl.IsContinuousRendering = false;
            for (const auto& render_pass : live_glsl.RenderPasses) {
                live_glsl.IsContinuousRendering |= glGetUniformLocation(render_pass.Program.Handle, "time") != -1;
            }

            if (live_glsl.Args.Output.empty()) {
                if (live_glsl.IsContinuousRendering) {
                    ScreenLogRenderFrameStatus(ScreenLogInstance, live_glsl.WindowWidth * live_glsl.PixelDensity, sixty_fps, live_glsl.PixelDensity);
                }

                if (draw_gui) {
                    GUIRender();
                }
            }

            if (!live_glsl.Args.Output.empty()) {
                uint32_t width = live_glsl.WindowWidth * live_glsl.PixelDensity;
                uint32_t height = live_glsl.WindowHeight * live_glsl.PixelDensity;
                unsigned char* pixels = new unsigned char[3 * width * height];
                glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

                int res = stbi_write_png(live_glsl.Args.Output.c_str(), width, height, 3, pixels, 3 * width);
                delete[] pixels;
                if (res == 0) {
                    fprintf(stderr, "Failed to write image to file: %s\n", live_glsl.Args.Output.c_str());
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
        }

        ScreenLogRender(ScreenLogInstance, live_glsl.PixelDensity);

        glfwSwapBuffers(live_glsl.GLFWWindowHandle);
        if (live_glsl.IsContinuousRendering)
            glfwPollEvents();
        else
            glfwWaitEvents();
    }

    return EXIT_SUCCESS;
}

void OnFileChange(void* user_data, const char* file) {
    ShaderFileChanged.store(true);
    glfwPostEmptyEvent();
}

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
    if (!ArgumentsParse(argc, argv, args)) {
        return EXIT_FAILURE;
    }

    FileWatcher = FileWatcherCreate(OnFileChange, nullptr, 16);

    ShaderFileChanged.store(false);
    LiveGLSLInstance = LiveGLSLCreate(args);
    if (!LiveGLSLInstance) {
        return EXIT_FAILURE;
    }

    bool res = LiveGLSLRender(*LiveGLSLInstance);
    LiveGLSLDestroy(LiveGLSLInstance);
    FileWatcherDestroy(FileWatcher);

    glfwTerminate();

#ifdef _WIN32
    delete[] argv;
#endif

    return res;
}

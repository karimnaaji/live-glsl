#include "liveglsl.h"

#include "utils.h"
#include "shaderparser.h"

#include <GLFW/glfw3.h>
#include <atomic>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#endif

void OnShaderChange(void* user_data, const char* file) {
    LiveGLSL* live_glsl = (LiveGLSL*)user_data;

    live_glsl->ShaderFileChanged.store(true);

    glfwPostEmptyEvent();
}

void ReloadShaderIfChanged(LiveGLSL* live_glsl, std::string path, bool first_load = false) {
    if (!live_glsl->ShaderFileChanged && !first_load) {
        return;
    }

    std::string read_file_error;
    std::vector<RenderPass> render_passes;
    std::vector<std::string> watches;
    
    GUIClearLog(live_glsl->GUI);

    if (!ShaderParserParse(live_glsl->BasePath, path, watches, render_passes, live_glsl->GUIComponents, read_file_error)) {
        if (!read_file_error.empty()) {
            GUILog(live_glsl->GUI, read_file_error);
        }
    } else {
        std::string error;
        live_glsl->ShaderCompiled = RenderPassCreate(render_passes, error);

        if (live_glsl->ShaderCompiled) {
            RenderPassDestroy(live_glsl->RenderPasses);

            live_glsl->RenderPasses = render_passes;

            if (live_glsl->FileWatcher) {
                FileWatcherRemoveAllWatches(live_glsl->FileWatcher);

                for (const auto& watch : watches) {
                    FileWatcherAddWatch(live_glsl->FileWatcher, watch.c_str());
                }
            }
        }

        glfwPostEmptyEvent();
    }

    live_glsl->ShaderFileChanged.store(false);
}

void MainLoop(void* user_data) {
    LiveGLSL* live_glsl = (LiveGLSL*)user_data;
    ReloadShaderIfChanged(live_glsl, live_glsl->ShaderPath);

#ifdef EMSCRIPTEN
    double width, height;
    emscripten_get_element_css_size("#canvas", &width, &height);
    live_glsl->WindowWidth = width;
    live_glsl->WindowHeight = height;
    live_glsl->PixelDensity = emscripten_get_device_pixel_ratio();
    emscripten_set_canvas_element_size("#canvas", width, height);
    GUIResize(live_glsl->GUI, width, height);
#endif

    double x, y;
    glfwGetCursorPos(live_glsl->GLFWWindowHandle, &x, &y);
    x *= live_glsl->PixelDensity;
    y *= live_glsl->PixelDensity;

    int mouse_left_state = glfwGetMouseButton(live_glsl->GLFWWindowHandle, GLFW_MOUSE_BUTTON_LEFT);

    if (live_glsl->ShaderCompiled) {
        for (GUIComponent& gui_component : live_glsl->GUIComponents) {
            gui_component.IsInUse = false;
            for (const auto& render_pass : live_glsl->RenderPasses) {
                GLuint uniform_location = glGetUniformLocation(render_pass.Program.Handle, gui_component.UniformName.c_str());
                gui_component.IsInUse |= uniform_location != -1;
            }
        }
    }

    std::vector<GUITexture> textures;
    for (const auto& render_pass : live_glsl->RenderPasses) {
        if (render_pass.TextureId) {
            GUITexture guiTexture;
            guiTexture.Width = render_pass.Width;
            guiTexture.Height = render_pass.Height;
            guiTexture.Id = render_pass.TextureId;
            textures.push_back(guiTexture);
        }
        for (const auto& texture : render_pass.Textures) {
            GUITexture guiTexture;
            guiTexture.Width = texture.Width;
            guiTexture.Height = texture.Height;
            guiTexture.Id = texture.Id;
            textures.push_back(guiTexture);
        }
    }

    GUINewFrame(live_glsl->GUI, live_glsl->GUIComponents, textures);

    if (live_glsl->ShaderCompiled) {
        for (const auto& render_pass : live_glsl->RenderPasses) {
            uint32_t width = render_pass.IsMain ? live_glsl->WindowWidth : render_pass.Width;
            uint32_t height = render_pass.IsMain ? live_glsl->WindowHeight : render_pass.Height;

            width *= live_glsl->PixelDensity;
            height *= live_glsl->PixelDensity;

            glBindFramebuffer(GL_FRAMEBUFFER, render_pass.FBO);
            glClear(GL_COLOR_BUFFER_BIT);

            assert(render_pass.Program.Handle != 0);
            glUseProgram(render_pass.Program.Handle);

            glViewport(0, 0, width, height);

            glUniform2f(glGetUniformLocation(render_pass.Program.Handle, "resolution"), width, height);
            glUniform1f(glGetUniformLocation(render_pass.Program.Handle, "time"), glfwGetTime());
            glUniform1f(glGetUniformLocation(render_pass.Program.Handle, "pixel_ratio"), live_glsl->PixelDensity);
            // Mouse position, x relative to left, y relative to top
            glUniform3f(glGetUniformLocation(render_pass.Program.Handle, "mouse"), x, y, mouse_left_state == GLFW_PRESS ? 1.0f : 0.0f);

            int texture_unit = 0;

            if (!render_pass.Input.empty()) {
                for (const auto& other : live_glsl->RenderPasses) {
                    if (other.Output == render_pass.Input) {
                        glActiveTexture(GL_TEXTURE0 + texture_unit);
                        glBindTexture(GL_TEXTURE_2D, other.TextureId);
                        glUniform1i(glGetUniformLocation(render_pass.Program.Handle, render_pass.Input.c_str()), texture_unit);
                        glUniform2f(glGetUniformLocation(render_pass.Program.Handle, (render_pass.Input + "_resolution").c_str()), other.Width, other.Height);
                        ++texture_unit;
                    }
                }
            }

            for (const GUIComponent& gui_component : live_glsl->GUIComponents) {
                GLuint uniform_location = glGetUniformLocation(render_pass.Program.Handle, gui_component.UniformName.c_str());
                switch (gui_component.UniformType) {
                    case EGUIUniformTypeFloat:
                        glUniform1f(uniform_location, gui_component.Data[0]);
                        break;
                    case EGUIUniformTypeVec2:
                        glUniform2f(uniform_location, gui_component.Data[0], gui_component.Data[1]);
                        break;
                    case EGUIUniformTypeVec3:
                        glUniform3f(uniform_location, gui_component.Data[0], gui_component.Data[1], gui_component.Data[2]);
                        break;
                    case EGUIUniformTypeVec4:
                        glUniform4f(uniform_location, gui_component.Data[0], gui_component.Data[1], gui_component.Data[2], gui_component.Data[3]);
                        break;
                }
            }

            for (const Texture& texture : render_pass.Textures) {
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(GL_TEXTURE_2D, texture.Id);
                glUniform1i(glGetUniformLocation(render_pass.Program.Handle, texture.Binding.c_str()), texture_unit);
                glUniform2f(glGetUniformLocation(render_pass.Program.Handle, (texture.Binding + "_resolution").c_str()), texture.Width, texture.Height);
                ++texture_unit;
            }

#ifndef EMSCRIPTEN
            glBindVertexArray(live_glsl->VaoId);
#endif
            glBindBuffer(GL_ARRAY_BUFFER, live_glsl->VertexBufferId);
            for (auto& render_pass : live_glsl->RenderPasses) {
                if (render_pass.IsMain) {
                    GLint position_attrib = glGetAttribLocation(render_pass.Program.Handle, "position");
                    glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
                    glEnableVertexAttribArray(position_attrib);
                }
            }

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        live_glsl->IsContinuousRendering = false;
        for (const auto& render_pass : live_glsl->RenderPasses) {
            live_glsl->IsContinuousRendering |= glGetUniformLocation(render_pass.Program.Handle, "time") != -1;
        }

        if (!live_glsl->Args.Output.empty()) {
#ifndef EMSCRIPTEN
            uint32_t width = live_glsl->WindowWidth * live_glsl->PixelDensity;
            uint32_t height = live_glsl->WindowHeight * live_glsl->PixelDensity;
            unsigned char* pixels = new unsigned char[3 * width * height];
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

            int res = stbi_write_png(live_glsl->Args.Output.c_str(), width, height, 3, pixels, 3 * width);
            delete[] pixels;
            if (res == 0) {
                fprintf(stderr, "Failed to write image to file: %s\n", live_glsl->Args.Output.c_str());
            }
            return;
#endif
        }
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    GUIRender(live_glsl->GUI);

    glfwSwapBuffers(live_glsl->GLFWWindowHandle);

    if (live_glsl->IsContinuousRendering) {
        glfwPollEvents();
    } else {
        glfwWaitEvents();
    }
}

LiveGLSL* LiveGLSLCreate(const Arguments& args) {
    LiveGLSL* live_glsl = new LiveGLSL();

    live_glsl->ShaderFileChanged.store(false);
#ifdef EMSCRIPTEN
    live_glsl->FileWatcher = nullptr;
#else
    live_glsl->FileWatcher = FileWatcherCreate(OnShaderChange, live_glsl, 16);
#endif
    live_glsl->ShaderCompiled = false;
    live_glsl->ShaderPath = args.Input;
    live_glsl->WindowWidth = args.Width;
    live_glsl->WindowHeight = args.Height;
    live_glsl->IsContinuousRendering = false;
    live_glsl->Args = args;
    live_glsl->BasePath = ExtractBasePath(args.Input);

    if (live_glsl->Args.EnableIni) {
        std::string shader_name = ExtractFilenameWithoutExt(args.Input);
        GUIComponentLoad(live_glsl->BasePath + "/" + shader_name + ".ini", live_glsl->GUIComponents);
    }

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
            GUIResize(live_glsl->GUI, window_width, window_height);
        });

        glfwSetKeyCallback(live_glsl->GLFWWindowHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GL_TRUE);
            GUIKeyCallback(window, key, scancode, action, mods);
        });

        glfwSetDropCallback(live_glsl->GLFWWindowHandle, [](GLFWwindow* window, int count, const char** paths) {
            if (count >= 0) {
                LiveGLSL* live_glsl = static_cast<LiveGLSL*>(glfwGetWindowUserPointer(window));
                std::string path = std::string(paths[0]);
                live_glsl->ShaderPath = path;
                live_glsl->Args.Input = path;
                live_glsl->BasePath = ExtractBasePath(path);

                ReloadShaderIfChanged(live_glsl, path, true);
            }
        });
        
        glfwSetMouseButtonCallback(live_glsl->GLFWWindowHandle, [](GLFWwindow* window, int button, int action, int mods) {
            LiveGLSL* live_glsl = static_cast<LiveGLSL*>(glfwGetWindowUserPointer(window));
            GUIMouseButtonCallback(live_glsl->GUI, button, action, mods);
        });

        int fb_width = 0;
        int fb_height = 0;
        glfwGetFramebufferSize(live_glsl->GLFWWindowHandle, &fb_width, &fb_height);
        live_glsl->PixelDensity = (float)fb_width / (float)live_glsl->WindowWidth;
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);
    }

    live_glsl->GUI = GUIInit(live_glsl->GLFWWindowHandle, args.Width, args.Height);

    ReloadShaderIfChanged(live_glsl, args.Input, true);

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

#ifndef EMSCRIPTEN
        glGenVertexArrays(1, &live_glsl->VaoId);
        glBindVertexArray(live_glsl->VaoId);
#endif
        glGenBuffers(1, &live_glsl->VertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, live_glsl->VertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    return live_glsl;
}

void LiveGLSLDestroy(LiveGLSL* live_glsl) {
    if (live_glsl->VertexBufferId) {
        glDeleteBuffers(1, &live_glsl->VertexBufferId);
    }

#ifndef EMSCRIPTEN
    if (live_glsl->VaoId) {
        glDeleteVertexArrays(1, &live_glsl->VaoId);
    }
#endif

    if (live_glsl->Args.EnableIni) {
        std::string shader_name = ExtractFilenameWithoutExt(live_glsl->ShaderPath);
        GUIComponentSave(live_glsl->BasePath + "/" + shader_name + ".ini", live_glsl->GUIComponents);
    }

    RenderPassDestroy(live_glsl->RenderPasses);
    if (live_glsl->FileWatcher) {
        FileWatcherDestroy(live_glsl->FileWatcher);
    }
    GUIDestroy(live_glsl->GUI);

    delete live_glsl;

    glfwTerminate();
}

int LiveGLSLRender(LiveGLSL* live_glsl) {
#ifdef EMSCRIPTEN
    emscripten_set_main_loop_arg(&MainLoop, live_glsl, 0, 1);
#else
    while (!glfwWindowShouldClose(live_glsl->GLFWWindowHandle)) {
        MainLoop(live_glsl);
    }
#endif
    return EXIT_SUCCESS;
}

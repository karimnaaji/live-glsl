#include "liveglsl.h"

#include "utils.h"
#include "screenlog.h"
#include "shaderparser.h"

#include <GLFW/glfw3.h>
#include <atomic>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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

    if (!ShaderParserParse(live_glsl->BasePath, path, watches, render_passes, live_glsl->GUIComponents, read_file_error)) {
        if (!read_file_error.empty()) {
            ScreenLogBuffer(live_glsl->ScreenLogInstance, read_file_error.c_str());
        }
    } else {
        RenderPassDestroy(live_glsl->RenderPasses);

        live_glsl->ShaderCompiled = RenderPassCreate(render_passes, live_glsl->ScreenLogInstance);
        live_glsl->RenderPasses = render_passes;

        FileWatcherRemoveAllWatches(live_glsl->FileWatcher);

        for (const auto& watch : watches) {
            FileWatcherAddWatch(live_glsl->FileWatcher, watch.c_str());
        }

        glfwPostEmptyEvent();
    }

    live_glsl->ShaderFileChanged.store(false);
}

LiveGLSL* LiveGLSLCreate(const Arguments& args) {
    LiveGLSL* live_glsl = new LiveGLSL();

    live_glsl->ShaderFileChanged.store(false);
    live_glsl->FileWatcher = FileWatcherCreate(OnShaderChange, live_glsl, 16);
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

                ReloadShaderIfChanged(live_glsl, path, true);
            }
        });

        int fb_width = 0;
        int fb_height = 0;
        glfwGetFramebufferSize(live_glsl->GLFWWindowHandle, &fb_width, &fb_height);
        live_glsl->PixelDensity = (float)fb_width / (float)live_glsl->WindowWidth;
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);
    }

    live_glsl->ScreenLogInstance = ScreenLogCreate(live_glsl->PixelDensity);

    GUIInit(live_glsl->GLFWWindowHandle);

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

        glGenVertexArrays(1, &live_glsl->VaoId);
        glBindVertexArray(live_glsl->VaoId);
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

    if (live_glsl->VaoId) {
        glDeleteVertexArrays(1, &live_glsl->VaoId);
    }

    RenderPassDestroy(live_glsl->RenderPasses);
    ScreenLogDestroy(live_glsl->ScreenLogInstance);
    FileWatcherDestroy(live_glsl->FileWatcher);

    delete live_glsl;

    glfwTerminate();
}

int LiveGLSLRender(LiveGLSL* live_glsl) {
    double previous_time = glfwGetTime();
    uint32_t frame_count = 0;
    bool sixty_fps = false;

    while (!glfwWindowShouldClose(live_glsl->GLFWWindowHandle)) {
        ReloadShaderIfChanged(live_glsl, live_glsl->ShaderPath);

        double current_time = glfwGetTime();
        ++frame_count;
        if (current_time - previous_time >= 1.0) {
            sixty_fps = frame_count >= 60;
            frame_count = 0;
            previous_time = current_time;
        }

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

            std::vector<GUITexture> textures;
            for (const auto& render_pass : live_glsl->RenderPasses) {
                if (render_pass.TextureId) {
                    GUITexture guiTexture;
                    guiTexture.Width = render_pass.Width;
                    guiTexture.Height = render_pass.Height;
                    guiTexture.Id = (ImTextureID)(intptr_t)render_pass.TextureId;
                    textures.push_back(guiTexture);
                }
                for (const auto& texture : render_pass.Textures) {
                    GUITexture guiTexture;
                    guiTexture.Width = texture.Width;
                    guiTexture.Height = texture.Height;
                    guiTexture.Id = (ImTextureID)(intptr_t)texture.Id;
                    textures.push_back(guiTexture);
                }
            }

            bool draw_gui = GUINewFrame(live_glsl->GUIComponents, textures);

            for (const auto& render_pass : live_glsl->RenderPasses) {
                uint32_t width = render_pass.IsMain ? live_glsl->WindowWidth : render_pass.Width;
                uint32_t height = render_pass.IsMain ? live_glsl->WindowHeight : render_pass.Height;

                width *= live_glsl->PixelDensity;
                height *= live_glsl->PixelDensity;

                glBindFramebuffer(GL_FRAMEBUFFER, render_pass.FBO);
                glClear(GL_COLOR_BUFFER_BIT);

                assert(render_pass.Program.Handle != 0);
                glUseProgram(render_pass.Program.Handle);

                glBindBuffer(GL_ARRAY_BUFFER, live_glsl->VertexBufferId);
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

                for (const Texture& texture : render_pass.Textures) {
                    glActiveTexture(GL_TEXTURE0 + texture_unit);
                    glBindTexture(GL_TEXTURE_2D, texture.Id);
                    glUniform1i(glGetUniformLocation(render_pass.Program.Handle, texture.Binding.c_str()), texture_unit);
                    glUniform2f(glGetUniformLocation(render_pass.Program.Handle, (texture.Binding + "_resolution").c_str()), texture.Width, texture.Height);
                    ++texture_unit;
                }

                glBindVertexArray(live_glsl->VaoId);

                for (auto& render_pass : live_glsl->RenderPasses) {
                    if (render_pass.IsMain) {
                        GLint position_attrib = glGetAttribLocation(render_pass.Program.Handle, "position");
                        glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
                        glEnableVertexAttribArray(position_attrib);
                    }
                }

                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            live_glsl->IsContinuousRendering = false;
            for (const auto& render_pass : live_glsl->RenderPasses) {
                live_glsl->IsContinuousRendering |= glGetUniformLocation(render_pass.Program.Handle, "time") != -1;
            }

            if (live_glsl->Args.Output.empty()) {
                if (live_glsl->IsContinuousRendering) {
                    ScreenLogRenderFrameStatus(live_glsl->ScreenLogInstance, live_glsl->WindowWidth * live_glsl->PixelDensity, sixty_fps, live_glsl->PixelDensity);
                }

                if (draw_gui) {
                    GUIRender();
                }
            }

            if (!live_glsl->Args.Output.empty()) {
                uint32_t width = live_glsl->WindowWidth * live_glsl->PixelDensity;
                uint32_t height = live_glsl->WindowHeight * live_glsl->PixelDensity;
                unsigned char* pixels = new unsigned char[3 * width * height];
                glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

                int res = stbi_write_png(live_glsl->Args.Output.c_str(), width, height, 3, pixels, 3 * width);
                delete[] pixels;
                if (res == 0) {
                    fprintf(stderr, "Failed to write image to file: %s\n", live_glsl->Args.Output.c_str());
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
        }

        ScreenLogRender(live_glsl->ScreenLogInstance, live_glsl->PixelDensity);

        glfwSwapBuffers(live_glsl->GLFWWindowHandle);

        if (live_glsl->IsContinuousRendering) {
            glfwPollEvents();
        } else {
            glfwWaitEvents();
        }
    }

    return EXIT_SUCCESS;
}

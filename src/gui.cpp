#include "gui.h"

#include "utils.h"
#include "arial.ttf.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <sstream>

extern "C" {
#include <microui/microui.h>
#include <microui/atlas.inl>
}

#define BUFFER_SIZE 16384

struct GUI {
    ShaderProgram Program;
    mu_Context* Ctx;
    GLFWwindow* Window;
    float UVArray[BUFFER_SIZE *  8];
    float VertexArray[BUFFER_SIZE *  8];
    uint8_t ColorArray[BUFFER_SIZE * 16];
    uint32_t IndexArray[BUFFER_SIZE *  6];
    GLuint AtlasId;
    GLuint VertexBuffer;
    GLuint UVBuffer;
    GLuint ColorBuffer;
    GLuint IndexBuff;
    int QuadCount;
    int Width;
    int Height;
    int CursorX;
    int CursorY;
    std::string Log;
};

static const char key_map[512] = {
    [GLFW_KEY_LEFT_SHIFT]    = MU_KEY_SHIFT,
    [GLFW_KEY_RIGHT_SHIFT]   = MU_KEY_SHIFT,
    [GLFW_KEY_LEFT_CONTROL]  = MU_KEY_CTRL,
    [GLFW_KEY_RIGHT_CONTROL] = MU_KEY_CTRL,
    [GLFW_KEY_LEFT_ALT]      = MU_KEY_ALT,
    [GLFW_KEY_RIGHT_ALT]     = MU_KEY_ALT,
    [GLFW_KEY_ENTER]         = MU_KEY_RETURN,
    [GLFW_KEY_BACKSPACE]     = MU_KEY_BACKSPACE,
};

static const char mouse_map[512] = {
    [GLFW_MOUSE_BUTTON_LEFT]   = MU_MOUSE_LEFT,
    [GLFW_MOUSE_BUTTON_RIGHT]  = MU_MOUSE_RIGHT,
    [GLFW_MOUSE_BUTTON_MIDDLE] = MU_MOUSE_MIDDLE
};

static void Render(GUI* gui) {
    if (gui->QuadCount == 0) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    glUseProgram(gui->Program.Handle);

    glBindBuffer(GL_ARRAY_BUFFER, gui->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gui->VertexArray), gui->VertexArray, GL_STREAM_DRAW);
    GLint position_attrib = glGetAttribLocation(gui->Program.Handle, "position");
    glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(position_attrib);

    glBindBuffer(GL_ARRAY_BUFFER, gui->UVBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gui->UVArray), gui->UVArray, GL_STREAM_DRAW);
    GLint uv_attrib = glGetAttribLocation(gui->Program.Handle, "uv");
    glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(uv_attrib);

    glBindBuffer(GL_ARRAY_BUFFER, gui->ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gui->ColorArray), gui->ColorArray, GL_STREAM_DRAW);
    GLint color_attrib = glGetAttribLocation(gui->Program.Handle, "color");
    glVertexAttribPointer(color_attrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(uint8_t), 0);
    glEnableVertexAttribArray(color_attrib);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui->IndexBuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gui->IndexArray), gui->IndexArray, GL_STREAM_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gui->AtlasId);
    glUniform1i(glGetUniformLocation(gui->Program.Handle, "atlas"), 0);

    glm::mat4 proj = glm::ortho(0.0f, (float)gui->Width, 0.0f, (float)gui->Height, -1.0f, 1.0f);
    // Flip the y-axis of the projection matrix
    glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
    proj = flipY * proj;
    glUniformMatrix4fv(glGetUniformLocation(gui->Program.Handle, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

    glDrawElements(GL_TRIANGLES, gui->QuadCount * 6, GL_UNSIGNED_INT, 0);

    gui->QuadCount = 0;
}

void GUIKeyCallback(HGUI handle, int key, int scancode, int action, int mods) {
    GUI* gui = (GUI*)handle;
    GLFWwindow* window = gui->Window;
    if (action == GLFW_PRESS) {
        mu_input_keydown(gui->Ctx, key_map[key]);
    }
    if (action == GLFW_RELEASE) {
        mu_input_keyup(gui->Ctx, key_map[key]);
    }
}

void GUIMouseButtonCallback(HGUI handle, int button, int action, int mods) {
    GUI* gui = (GUI*)handle;
    GLFWwindow* window = gui->Window;

    if (action == GLFW_PRESS) {
        mu_input_mousedown(gui->Ctx, gui->CursorX, gui->CursorY, mouse_map[button]);
        
    }
    if (action == GLFW_RELEASE) {
        mu_input_mouseup(gui->Ctx, gui->CursorX, gui->CursorY, mouse_map[button]);
    }
}

static void PushQuad(GUI* gui, mu_Rect dst, mu_Rect src, mu_Color color) {
    if (gui->QuadCount == BUFFER_SIZE) {
        Render(gui);
    }

    int texvert_idx = gui->QuadCount * 8;
    int color_idx = gui->QuadCount * 16;
    int element_idx = gui->QuadCount * 4;
    int index_idx = gui->QuadCount * 6;
    gui->QuadCount++;

    float x = src.x / (float) ATLAS_WIDTH;
    float y = src.y / (float) ATLAS_HEIGHT;
    float w = src.w / (float) ATLAS_WIDTH;
    float h = src.h / (float) ATLAS_HEIGHT;
    gui->UVArray[texvert_idx + 0] = x;
    gui->UVArray[texvert_idx + 1] = y;
    gui->UVArray[texvert_idx + 2] = x + w;
    gui->UVArray[texvert_idx + 3] = y;
    gui->UVArray[texvert_idx + 4] = x;
    gui->UVArray[texvert_idx + 5] = y + h;
    gui->UVArray[texvert_idx + 6] = x + w;
    gui->UVArray[texvert_idx + 7] = y + h;

    gui->VertexArray[texvert_idx + 0] = dst.x;
    gui->VertexArray[texvert_idx + 1] = dst.y;
    gui->VertexArray[texvert_idx + 2] = dst.x + dst.w;
    gui->VertexArray[texvert_idx + 3] = dst.y;
    gui->VertexArray[texvert_idx + 4] = dst.x;
    gui->VertexArray[texvert_idx + 5] = dst.y + dst.h;
    gui->VertexArray[texvert_idx + 6] = dst.x + dst.w;
    gui->VertexArray[texvert_idx + 7] = dst.y + dst.h;

    memcpy(gui->ColorArray + color_idx +  0, &color, 4);
    memcpy(gui->ColorArray + color_idx +  4, &color, 4);
    memcpy(gui->ColorArray + color_idx +  8, &color, 4);
    memcpy(gui->ColorArray + color_idx + 12, &color, 4);

    gui->IndexArray[index_idx + 0] = element_idx + 0;
    gui->IndexArray[index_idx + 1] = element_idx + 1;
    gui->IndexArray[index_idx + 2] = element_idx + 2;
    gui->IndexArray[index_idx + 3] = element_idx + 2;
    gui->IndexArray[index_idx + 4] = element_idx + 3;
    gui->IndexArray[index_idx + 5] = element_idx + 1;
}

static void DrawRect(GUI* gui, mu_Rect rect, mu_Color color) {
    PushQuad(gui, rect, atlas[ATLAS_WHITE], color);
}

static void DrawText(GUI* gui, const char *text, mu_Vec2 pos, mu_Color color) {
    mu_Rect dst = { pos.x, pos.y, 0, 0 };
    for (const char *p = text; *p; p++) {
        if ((*p & 0xc0) == 0x80) {
            continue;
        }
        int chr = mu_min((unsigned char) *p, 127);
        mu_Rect src = atlas[ATLAS_FONT + chr];
        dst.w = src.w;
        dst.h = src.h;
        PushQuad(gui, dst, src, color);
        dst.x += dst.w;
    }
}

static void DrawIcon(GUI* gui, int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  PushQuad(gui, mu_rect(x, y, src.w, src.h), src, color);
}

static void ClipRect(GUI* gui, mu_Rect rect) {
    Render(gui);
    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x, gui->Height - (rect.y + rect.h), rect.w, rect.h);
}

static int GetTextWidth(mu_Font font, const char *text, int len) {
    if (len == -1) {
        len = strlen(text);
    }

    int res = 0;
    for (const char *p = text; *p && len--; p++) {
        if ((*p & 0xc0) == 0x80) {
            continue;
        }
        int chr = mu_min((unsigned char) *p, 127);
        res += atlas[ATLAS_FONT + chr].w;
    }
    return res;
}

static int GetTextHeight(mu_Font font) {
    return 18;
}

HGUI GUIInit(GLFWwindow* window_handle, int width, int height) {
    GUI* gui = new GUI;

    gui->Window = window_handle;
    gui->Width = width;
    gui->Height = height;

    uint32_t rgba8_size = ATLAS_WIDTH * ATLAS_HEIGHT * 4;
    uint32_t* rgba8_pixels = new uint32_t[rgba8_size];
    for (int y = 0; y < ATLAS_HEIGHT; y++) {
        for (int x = 0; x < ATLAS_WIDTH; x++) {
            int index = y*ATLAS_WIDTH + x;
            rgba8_pixels[index] = 0x00FFFFFF | ((uint32_t)atlas_texture[index] << 24);
        }
    }

    glGenTextures(1, &gui->AtlasId);
    glBindTexture(GL_TEXTURE_2D, gui->AtlasId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba8_pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    delete[] rgba8_pixels;

    const GLchar* vertex_shader = R"END(
    in vec2 position;
    in vec2 uv;
    in vec4 color;
    uniform mat4 proj;
    out vec2 v_uv;
    out vec4 v_color;
    void main() {
        gl_Position = proj * vec4(position, 0.0, 1.0);
        v_color = color;
        v_uv = uv;
    }
    )END";

    const GLchar* fragment_shader = R"END(
    in vec2 v_uv;
    in vec4 v_color;
    uniform sampler2D atlas;
    out vec4 outColor;
    void main() {
        outColor = texture(atlas, v_uv) * v_color;
    }
    )END";

    std::string error;
    if (!ShaderProgramCreate(gui->Program, fragment_shader, vertex_shader, error)) {
        printf("%s\n", error.c_str());
    }

    glGenBuffers(1, &gui->ColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gui->ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gui->ColorArray), nullptr, GL_STREAM_DRAW);

    glGenBuffers(1, &gui->UVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gui->UVBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gui->UVArray), nullptr, GL_STREAM_DRAW);

    glGenBuffers(1, &gui->VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gui->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gui->VertexArray), nullptr, GL_STREAM_DRAW);

    glGenBuffers(1, &gui->IndexBuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui->IndexBuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gui->IndexArray), nullptr, GL_STREAM_DRAW);

    gui->Ctx = new mu_Context();
    mu_init(gui->Ctx);

    gui->Ctx->text_width = GetTextWidth;
    gui->Ctx->text_height = GetTextHeight;
    
    gui->Ctx->style->colors[MU_COLOR_BORDER].a = 0;
    gui->Ctx->style->colors[MU_COLOR_WINDOWBG].a = 128;
    gui->Ctx->style->colors[MU_COLOR_BASE].a = 200;
    gui->Ctx->style->colors[MU_COLOR_BUTTON].r = 34;
    gui->Ctx->style->colors[MU_COLOR_BUTTON].g = 85;
    gui->Ctx->style->colors[MU_COLOR_BUTTON].b = 255;
    gui->Ctx->style->colors[MU_COLOR_BUTTON].a = 150;
    gui->Ctx->style->colors[MU_COLOR_BUTTONHOVER].r = 34;
    gui->Ctx->style->colors[MU_COLOR_BUTTONHOVER].g = 85;
    gui->Ctx->style->colors[MU_COLOR_BUTTONHOVER].b = 255;
    gui->Ctx->style->colors[MU_COLOR_BUTTONHOVER].a = 100;

    return gui;
}

void GUIResize(HGUI handle, int width, int height) {
    GUI* gui = (GUI*)handle;
    gui->Width = width;
    gui->Height = height;
}

void GUILog(HGUI handle, std::string log) {
    GUI* gui = (GUI*)handle;
    if (!gui->Log.empty()) {
        gui->Log += "\n";
    }
    gui->Log += log;
}

void GUIClearLog(HGUI handle) {
    GUI* gui = (GUI*)handle;
    gui->Log = "";
}

bool GUINewFrame(HGUI handle, std::vector<GUIComponent>& gui_components, std::vector<GUITexture> textures) {
    if (gui_components.empty() && textures.empty()) {
        return false;
    }

    uint32_t components_in_use = 0;
    for (const GUIComponent& component : gui_components) {
        if (component.IsInUse)
            ++components_in_use;
    }

    if (components_in_use == 0 && textures.empty()) {
        return false;
    }

    GUI* gui = (GUI*)handle;
    int window_w = 300;

    mu_begin(gui->Ctx);
    if (mu_begin_window_ex(gui->Ctx, "", mu_rect(0, 0, window_w, gui->Height), MU_OPT_NOTITLE | MU_OPT_FORCE_RESIZE)) {
        if (!gui->Log.empty()) {
            mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, -1);
            mu_begin_panel(gui->Ctx, "Log Output");
            mu_Container* panel = mu_get_current_container(gui->Ctx);
            mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, -1);
            mu_text(gui->Ctx, gui->Log.c_str());
            mu_end_panel(gui->Ctx);
        } else {
            int label_text_w = 150;

            int column_1_w = (int)(window_w * 0.5f);
            int column_2_w = (int)(window_w * 0.5f) - 15;

            int component_1_w = window_w - label_text_w;
            int component_2_w = (component_1_w / 2.0f) - gui->Ctx->style->padding * 0.5f;
            int component_3_w = (component_1_w / 3.0f) - gui->Ctx->style->padding * 0.5f;
            int component_4_w = (component_1_w / 4.0f) - gui->Ctx->style->padding * 0.5f;
            int component_5_w = (component_1_w / 5.0f) - gui->Ctx->style->padding * 0.5f;

            for (GUIComponent& component : gui_components) {
                if (!component.IsInUse) continue;
                switch (component.Type) {
                    case EGUIComponentTypeSlider1:
                        mu_layout_row(gui->Ctx, 2, (int[]) { column_1_w, column_2_w }, 0);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec1, component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_layout_end_column(gui->Ctx);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, 0);
                        mu_label(gui->Ctx, component.UniformName.c_str());
                        mu_layout_end_column(gui->Ctx);
                        
                        break;
                    case EGUIComponentTypeSlider2:
                        mu_layout_row(gui->Ctx, 2, (int[]) { column_1_w, column_2_w }, 0);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 2, (int[]) { component_2_w, -1 }, 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec2[0], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec2[1], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_layout_end_column(gui->Ctx);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, 0);
                        mu_label(gui->Ctx, component.UniformName.c_str());
                        mu_layout_end_column(gui->Ctx);
                        
                        break;
                    case EGUIComponentTypeSlider3:
                        mu_layout_row(gui->Ctx, 2, (int[]) { column_1_w, column_2_w }, 0);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 3, (int[]) { component_3_w, component_3_w, -1 }, 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec3[0], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec3[1], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec3[2], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_layout_end_column(gui->Ctx);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, 0);
                        mu_label(gui->Ctx, component.UniformName.c_str());
                        mu_layout_end_column(gui->Ctx);
                        break;
                    case EGUIComponentTypeSlider4:
                        mu_layout_row(gui->Ctx, 2, (int[]) { column_1_w, column_2_w }, 0);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 4, (int[]) { component_4_w, component_4_w, component_4_w, -1 }, 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[0], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[1], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[2], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[3], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", 0);
                        mu_layout_end_column(gui->Ctx);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, 0);
                        mu_label(gui->Ctx, component.UniformName.c_str());
                        mu_layout_end_column(gui->Ctx);
                        break;
                    case EGUIComponentTypeColor3:
                    {
                        mu_layout_row(gui->Ctx, 2, (int[]) { column_1_w, column_2_w }, 0);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 4, (int[]) { component_4_w, component_4_w, component_4_w, -1 }, 0);
                        
                        component.Vec3 *= 255.0f;
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec3[0], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec3[1], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec3[2], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        component.Vec3 /= 255.0f;
                        
                        mu_Color color;
                        color.r = (uint8_t)(component.Vec3.r * 255.0f);
                        color.g = (uint8_t)(component.Vec3.g * 255.0f);
                        color.b = (uint8_t)(component.Vec3.b * 255.0f);
                        color.a = 255;
                        mu_draw_rect(gui->Ctx, mu_layout_next(gui->Ctx), color);
                        mu_layout_end_column(gui->Ctx);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, 0);
                        mu_label(gui->Ctx, component.UniformName.c_str());
                        mu_layout_end_column(gui->Ctx);
                        
                        break;
                    }
                    case EGUIComponentTypeColor4:
                    {
                        mu_layout_row(gui->Ctx, 2, (int[]) { column_1_w, column_2_w }, 0);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 5, (int[]) { component_5_w, component_5_w, component_5_w, component_5_w, -1 }, 0);
                        
                        component.Vec4 *= 255.0f;
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[0], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[1], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[2], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        mu_slider_ex(gui->Ctx, (float*)&component.Vec4[3], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        component.Vec4 /= 255.0f;
                        
                        mu_Color color;
                        color.r = (uint8_t)(component.Vec4.r * 255.0f);
                        color.g = (uint8_t)(component.Vec4.g * 255.0f);
                        color.b = (uint8_t)(component.Vec4.b * 255.0f);
                        color.a = (uint8_t)(component.Vec4.a * 255.0f);
                        mu_draw_rect(gui->Ctx, mu_layout_next(gui->Ctx), color);
                        mu_layout_end_column(gui->Ctx);
                        
                        mu_layout_begin_column(gui->Ctx);
                        mu_layout_row(gui->Ctx, 1, (int[]) { -1 }, 0);
                        mu_label(gui->Ctx, component.UniformName.c_str());
                        mu_layout_end_column(gui->Ctx);
                        
                        component.Vec4.r = color.r / 255.0f;
                        component.Vec4.g = color.g / 255.0f;
                        component.Vec4.b = color.b / 255.0f;
                        component.Vec4.a = color.a / 255.0f;
                        break;
                    }
                }
            }
        }
#if 0
        for (const auto& texture : textures) {
            float aspect = (float)texture.Width / (float)texture.Height;
            float max_width = ImGui::GetContentRegionAvailWidth();
            float height = max_width / aspect;

            ImGui::Image(texture.Id, ImVec2(max_width, height), ImVec2(0, 1), ImVec2(1, 0));
        }
#endif
        mu_end_window(gui->Ctx);
    }
    mu_end(gui->Ctx);

    return true;
}

void GUIRender(HGUI handle) {
    GUI* gui = (GUI*)handle;
    GLFWwindow* window = gui->Window;

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    mu_input_mousemove(gui->Ctx, x, y);
    gui->CursorX = x;
    gui->CursorY = y;

    mu_Command *cmd = NULL;
    while (mu_next_command(gui->Ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT: DrawText(gui, cmd->text.str, cmd->text.pos, cmd->text.color); break;
            case MU_COMMAND_RECT: DrawRect(gui, cmd->rect.rect, cmd->rect.color); break;
            case MU_COMMAND_ICON: DrawIcon(gui, cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
            case MU_COMMAND_CLIP: ClipRect(gui, cmd->clip.rect); break;
        }
    }

    Render(gui);
}

void GUIDestroy(HGUI handle) {
    GUI* gui = (GUI*)handle;
    delete gui->Ctx;
    ShaderProgramDestroy(gui->Program);
    delete gui;
}

bool GUIComponentLoad(const std::string& path, std::vector<GUIComponent>& out_components) {
    std::ifstream file(path);
    
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string uniformName;
        std::string type;
        iss >> uniformName >> type;
        GUIComponent comp;
        if (type == "s1") {
            float value;
            iss >> value;
            comp.Type = EGUIComponentTypeSlider1;
            comp.UniformName = uniformName;
            comp.Vec1 = value;
        } else if (type == "s2") {
            float x, y;
            iss >> x >> y;
            comp.Type = EGUIComponentTypeSlider2;
            comp.UniformName = uniformName;
            comp.Vec2 = {x, y};
        } else if (type == "s3") {
            float x, y, z;
            iss >> x >> y >> z;
            comp.Type = EGUIComponentTypeSlider3;
            comp.UniformName = uniformName;
            comp.Vec3 = {x, y, z};
        } else if (type == "s4") {
            float x, y, z, w;
            iss >> x >> y >> z >> w;
            comp.Type = EGUIComponentTypeSlider4;
            comp.UniformName = uniformName;
            comp.Vec4 = {x, y, z, w};
        } else if (type == "c3") {
            float x, y, z;
            iss >> x >> y >> z;
            comp.Type = EGUIComponentTypeColor3;
            comp.UniformName = uniformName;
            comp.Vec3 = {x, y, z};
        } else if (type == "c4") {
            float x, y, z, w;
            iss >> x >> y >> z >> w;
            comp.Type = EGUIComponentTypeColor4;
            comp.UniformName = uniformName;
            comp.Vec4 = {x, y, z, w};
        }
        out_components.push_back(comp);
    }
    
    file.close();
    return true;
}


bool GUIComponentSave(const std::string& path, const std::vector<GUIComponent>& components) {
    std::ofstream file(path);
    
    if (!file.is_open()) {
        return false;
    }
    
    for (const GUIComponent& component : components) {
        switch (component.Type) {
            case EGUIComponentTypeSlider1:
                file << component.UniformName << " s1 ";
                file << component.Vec1;
                file << std::endl;
                break;
            case EGUIComponentTypeSlider2:
                file << component.UniformName << " s2 ";
                file << component.Vec2.x << " ";
                file << component.Vec2.y;
                file << std::endl;
                break;
            case EGUIComponentTypeSlider3:
                file << component.UniformName << " s3 ";
                file << component.Vec3.x << " ";
                file << component.Vec3.y << " ";
                file << component.Vec3.z;
                file << std::endl;
                break;
            case EGUIComponentTypeSlider4:
                file << component.UniformName << " s4 ";
                file << component.Vec4.x << " ";
                file << component.Vec4.y << " ";
                file << component.Vec4.z << " ";
                file << component.Vec4.w;
                file << std::endl;
                break;
            case EGUIComponentTypeColor3:
                file << component.UniformName << " c3 ";
                file << component.Vec3.x << " ";
                file << component.Vec3.y << " ";
                file << component.Vec3.z;
                file << std::endl;
                break;
            case EGUIComponentTypeColor4:
                file << component.UniformName << " c4 ";
                file << component.Vec4.x << " ";
                file << component.Vec4.y << " ";
                file << component.Vec4.z << " ";
                file << component.Vec4.w;
                file << std::endl;
                break;
        }
    }
    
    file.close();
    return true;
}

bool GUIComponentParse(uint32_t line_number, const std::string& gui_component_line, const std::string& uniform_line, const std::vector<GUIComponent>& previous_components, GUIComponent& out_component, std::string& out_parse_error) {
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
        }
    } else {
        // Initialize the data with convenient defaults
        if (GUIIsSliderType(out_component.Type)) {
            out_component.SliderRange.Start = 0.0f;
            out_component.SliderRange.End = 1.0f;
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
    } else if (uniform_type == "vec4") {
        out_component.UniformType = EGUIUniformTypeVec4;
    } else {
        ReportError("GUI not supported for uniform type '" + uniform_type + "'");
        return false;
    }

    if (GUIComponents(out_component.Type) != GUIUniformVariableComponents(out_component.UniformType)) {
        ReportError("GUI component count does not match uniform component count");
        return false;
    }

    // Trim semicolon ';'
    while (uniform_tokens.back().back() == ';') {
        uniform_tokens.back().pop_back();
    }

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

#include "gui.h"

#include "utils.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader.h"

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
    mu_Rect Atlas[ATLAS_FONT+127+1];
    char KeyMap[512];
    char MouseMap[512];
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

    float proj[16] = {
        2.0f / gui->Width, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / gui->Height, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(gui->Program.Handle, "proj"), 1, GL_FALSE, proj);

    glDrawElements(GL_TRIANGLES, gui->QuadCount * 6, GL_UNSIGNED_INT, 0);

    gui->QuadCount = 0;
}

void GUIKeyCallback(HGUI handle, int key, int scancode, int action, int mods) {
    GUI* gui = (GUI*)handle;
    GLFWwindow* window = gui->Window;
    if (action == GLFW_PRESS) {
        mu_input_keydown(gui->Ctx, gui->KeyMap[key]);
    }
    if (action == GLFW_RELEASE) {
        mu_input_keyup(gui->Ctx, gui->KeyMap[key]);
    }
}

void GUIMouseButtonCallback(HGUI handle, int button, int action, int mods) {
    GUI* gui = (GUI*)handle;
    GLFWwindow* window = gui->Window;

    if (action == GLFW_PRESS) {
        mu_input_mousedown(gui->Ctx, gui->CursorX, gui->CursorY, gui->MouseMap[button]);
        
    }
    if (action == GLFW_RELEASE) {
        mu_input_mouseup(gui->Ctx, gui->CursorX, gui->CursorY, gui->MouseMap[button]);
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
    PushQuad(gui, rect, gui->Atlas[ATLAS_WHITE], color);
}

static void DrawText(GUI* gui, const char *text, mu_Vec2 pos, mu_Color color) {
    mu_Rect dst = { pos.x, pos.y, 0, 0 };
    for (const char *p = text; *p; p++) {
        if ((*p & 0xc0) == 0x80) {
            continue;
        }
        int chr = mu_min((unsigned char) *p, 127);
        mu_Rect src = gui->Atlas[ATLAS_FONT + chr];
        dst.w = src.w;
        dst.h = src.h;
        PushQuad(gui, dst, src, color);
        dst.x += dst.w;
    }
}

static void DrawIcon(GUI* gui, int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = gui->Atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  PushQuad(gui, mu_rect(x, y, src.w, src.h), src, color);
}

static void ClipRect(GUI* gui, mu_Rect rect) {
    Render(gui);
    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x, gui->Height - (rect.y + rect.h), rect.w, rect.h);
}

static int GetTextWidth(mu_Font font, const char *text, int len, void* user_data) {
    GUI* gui = (GUI*)user_data;
    if (len == -1) {
        len = strlen(text);
    }

    int res = 0;
    for (const char *p = text; *p && len--; p++) {
        if ((*p & 0xc0) == 0x80) {
            continue;
        }
        int chr = mu_min((unsigned char) *p, 127);
        res += gui->Atlas[ATLAS_FONT + chr].w;
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

    memset(gui->Atlas, 0x0, sizeof(gui->Atlas));
    gui->Atlas[ MU_ICON_CLOSE ] = { 88, 68, 16, 16 };
    gui->Atlas[ MU_ICON_CHECK ] = { 0, 0, 18, 18 };
    gui->Atlas[ MU_ICON_EXPANDED ] = { 118, 68, 7, 5 };
    gui->Atlas[ MU_ICON_COLLAPSED ] = { 113, 68, 5, 7 };
    gui->Atlas[ ATLAS_WHITE ] = { 125, 68, 3, 3 };
    gui->Atlas[ ATLAS_FONT+32 ] = { 84, 68, 2, 17 };
    gui->Atlas[ ATLAS_FONT+33 ] = { 39, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+34 ] = { 114, 51, 5, 17 };
    gui->Atlas[ ATLAS_FONT+35 ] = { 34, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+36 ] = { 28, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+37 ] = { 58, 0, 9, 17 };
    gui->Atlas[ ATLAS_FONT+38 ] = { 103, 0, 8, 17 };
    gui->Atlas[ ATLAS_FONT+39 ] = { 86, 68, 2, 17 };
    gui->Atlas[ ATLAS_FONT+40 ] = { 42, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+41 ] = { 45, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+42 ] = { 34, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+43 ] = { 40, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+44 ] = { 48, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+45 ] = { 51, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+46 ] = { 54, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+47 ] = { 124, 34, 4, 17 };
    gui->Atlas[ ATLAS_FONT+48 ] = { 46, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+49 ] = { 52, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+50 ] = { 58, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+51 ] = { 64, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+52 ] = { 70, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+53 ] = { 76, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+54 ] = { 82, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+55 ] = { 88, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+56 ] = { 94, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+57 ] = { 100, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+58 ] = { 57, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+59 ] = { 60, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+60 ] = { 106, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+61 ] = { 112, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+62 ] = { 118, 34, 6, 17 };
    gui->Atlas[ ATLAS_FONT+63 ] = { 119, 51, 5, 17 };
    gui->Atlas[ ATLAS_FONT+64 ] = { 18, 0, 10, 17 };
    gui->Atlas[ ATLAS_FONT+65 ] = { 41, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+66 ] = { 48, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+67 ] = { 55, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+68 ] = { 111, 0, 8, 17 };
    gui->Atlas[ ATLAS_FONT+69 ] = { 0, 35, 6, 17 };
    gui->Atlas[ ATLAS_FONT+70 ] = { 6, 35, 6, 17 };
    gui->Atlas[ ATLAS_FONT+71 ] = { 119, 0, 8, 17 };
    gui->Atlas[ ATLAS_FONT+72 ] = { 18, 17, 8, 17 };
    gui->Atlas[ ATLAS_FONT+73 ] = { 63, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+74 ] = { 66, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+75 ] = { 62, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+76 ] = { 12, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+77 ] = { 28, 0, 10, 17 };
    gui->Atlas[ ATLAS_FONT+78 ] = { 67, 0, 9, 17 };
    gui->Atlas[ ATLAS_FONT+79 ] = { 76, 0, 9, 17 };
    gui->Atlas[ ATLAS_FONT+80 ] = { 69, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+81 ] = { 85, 0, 9, 17 };
    gui->Atlas[ ATLAS_FONT+82 ] = { 76, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+83 ] = { 18, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+84 ] = { 24, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+85 ] = { 26, 17, 8, 17 };
    gui->Atlas[ ATLAS_FONT+86 ] = { 83, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+87 ] = { 38, 0, 10, 17 };
    gui->Atlas[ ATLAS_FONT+88 ] = { 90, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+89 ] = { 30, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+90 ] = { 36, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+91 ] = { 69, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+92 ] = { 124, 51, 4, 17 };
    gui->Atlas[ ATLAS_FONT+93 ] = { 72, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+94 ] = { 42, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+95 ] = { 15, 68, 4, 17 };
    gui->Atlas[ ATLAS_FONT+96 ] = { 48, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+97 ] = { 54, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+98 ] = { 97, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+99 ] = { 0, 52, 5, 17 };
    gui->Atlas[ ATLAS_FONT+100 ] = { 104, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+101 ] = { 60, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+102 ] = { 19, 68, 4, 17 };
    gui->Atlas[ ATLAS_FONT+103 ] = { 66, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+104 ] = { 111, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+105 ] = { 75, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+106 ] = { 78, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+107 ] = { 72, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+108 ] = { 81, 68, 3, 17 };
    gui->Atlas[ ATLAS_FONT+109 ] = { 48, 0, 10, 17 };
    gui->Atlas[ ATLAS_FONT+110 ] = { 118, 17, 7, 17 };
    gui->Atlas[ ATLAS_FONT+111 ] = { 0, 18, 7, 17 };
    gui->Atlas[ ATLAS_FONT+112 ] = { 7, 18, 7, 17 };
    gui->Atlas[ ATLAS_FONT+113 ] = { 14, 34, 7, 17 };
    gui->Atlas[ ATLAS_FONT+114 ] = { 23, 68, 4, 17 };
    gui->Atlas[ ATLAS_FONT+115 ] = { 5, 52, 5, 17 };
    gui->Atlas[ ATLAS_FONT+116 ] = { 27, 68, 4, 17 };
    gui->Atlas[ ATLAS_FONT+117 ] = { 21, 34, 7, 17 };
    gui->Atlas[ ATLAS_FONT+118 ] = { 78, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+119 ] = { 94, 0, 9, 17 };
    gui->Atlas[ ATLAS_FONT+120 ] = { 84, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+121 ] = { 90, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+122 ] = { 10, 68, 5, 17 };
    gui->Atlas[ ATLAS_FONT+123 ] = { 31, 68, 4, 17 };
    gui->Atlas[ ATLAS_FONT+124 ] = { 96, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+125 ] = { 35, 68, 4, 17 };
    gui->Atlas[ ATLAS_FONT+126 ] = { 102, 51, 6, 17 };
    gui->Atlas[ ATLAS_FONT+127 ] = { 108, 51, 6, 17 };

    memset(gui->KeyMap, 0x0, sizeof(gui->KeyMap));
    gui->KeyMap[GLFW_KEY_LEFT_SHIFT]    = MU_KEY_SHIFT;
    gui->KeyMap[GLFW_KEY_RIGHT_SHIFT]   = MU_KEY_SHIFT;
    gui->KeyMap[GLFW_KEY_LEFT_CONTROL]  = MU_KEY_CTRL;
    gui->KeyMap[GLFW_KEY_RIGHT_CONTROL] = MU_KEY_CTRL;
    gui->KeyMap[GLFW_KEY_LEFT_ALT]      = MU_KEY_ALT;
    gui->KeyMap[GLFW_KEY_RIGHT_ALT]     = MU_KEY_ALT;
    gui->KeyMap[GLFW_KEY_ENTER]         = MU_KEY_RETURN;
    gui->KeyMap[GLFW_KEY_BACKSPACE]     = MU_KEY_BACKSPACE;

    memset(gui->MouseMap, 0x0, sizeof(gui->MouseMap));
    gui->MouseMap[GLFW_MOUSE_BUTTON_LEFT]   = MU_MOUSE_LEFT;
    gui->MouseMap[GLFW_MOUSE_BUTTON_RIGHT]  = MU_MOUSE_RIGHT;
    gui->MouseMap[GLFW_MOUSE_BUTTON_MIDDLE] = MU_MOUSE_MIDDLE;

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
    precision mediump float;
    attribute vec2 position;
    attribute vec2 uv;
    attribute vec4 color;
    uniform mat4 proj;
    varying vec2 v_uv;
    varying vec4 v_color;
    void main() {
        gl_Position = proj * vec4(position, 0.0, 1.0);
        v_color = color;
        v_uv = uv;
    }
    )END";

    const GLchar* fragment_shader = R"END(
    precision mediump float;
    varying vec2 v_uv;
    varying vec4 v_color;
    uniform sampler2D atlas;
    
    void main() {
        gl_FragColor = texture2D(atlas, v_uv) * v_color;
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

    gui->Ctx->user_data = gui;

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
    int empty_width[1] = {-1};

    mu_begin(gui->Ctx);
    if (mu_begin_window_ex(gui->Ctx, "", mu_rect(0, 0, window_w, gui->Height), MU_OPT_NOTITLE | MU_OPT_FORCE_RESIZE)) {
        if (!gui->Log.empty()) {
            mu_layout_row(gui->Ctx, 1, empty_width, -1);
            mu_begin_panel(gui->Ctx, "Log Output");
            mu_Container* panel = mu_get_current_container(gui->Ctx);

            mu_layout_row(gui->Ctx, 1, empty_width, -1);
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
                if (!component.IsInUse) {
                    continue;
                }

                int column_widths[2] = {column_1_w, column_2_w};
                mu_layout_row(gui->Ctx, 2, column_widths, 0);
                mu_layout_begin_column(gui->Ctx);
                switch (component.Type) {
                    case EGUIComponentTypeSlider1: {
                        mu_layout_row(gui->Ctx, 1, empty_width, 0);
                        break;
                    }
                    case EGUIComponentTypeSlider2: {
                        int widths[2] = { component_2_w, -1 };
                        mu_layout_row(gui->Ctx, 2, widths, 0);
                        break;
                    }
                    case EGUIComponentTypeSlider3: {
                        int widths[3] = { component_3_w, component_3_w, -1 };
                        mu_layout_row(gui->Ctx, 3, widths, 0);
                        break;
                    }
                    case EGUIComponentTypeColor3:
                    case EGUIComponentTypeSlider4: {
                        int widths[4] = { component_4_w, component_4_w, component_4_w, -1 };
                        mu_layout_row(gui->Ctx, 4, widths, 0);
                        break;
                    }
                    case EGUIComponentTypeColor4: {
                        int widths[5] = { component_5_w, component_5_w, component_5_w, component_5_w, -1 };
                        mu_layout_row(gui->Ctx, 5, widths, 0);
                        break;
                    }
                }

                if (GUIIsSliderType(component.Type)) {
                    for (int i = 0; i < GUIComponents(component.Type); ++i) {
                        mu_slider_ex(gui->Ctx, &component.Data[i], component.SliderRange.Start, component.SliderRange.End, 0.01f, "%.2f", MU_OPT_ALIGNCENTER);
                    }
                } else {
                    for (int i = 0; i < GUIComponents(component.Type); ++i) {
                        component.Data[i] *= 255.0f;
                        mu_slider_ex(gui->Ctx, &component.Data[i], 0.0, 255.0f, 0, "%.0f", MU_OPT_ALIGNCENTER);
                        component.Data[i] /= 255.0f;
                    }
                    
                    mu_Color color;
                    color.r = (uint8_t)(component.Data[0] * 255.0f);
                    color.g = (uint8_t)(component.Data[1] * 255.0f);
                    color.b = (uint8_t)(component.Data[2] * 255.0f);
                    color.a = GUIComponents(component.Type) == 4 ? (uint8_t)(component.Data[3] * 255.0f) : 255;
                    mu_draw_rect(gui->Ctx, mu_layout_next(gui->Ctx), color);
                }
                mu_layout_end_column(gui->Ctx);

                mu_layout_begin_column(gui->Ctx);
                mu_layout_row(gui->Ctx, 1, empty_width, 0);
                mu_label(gui->Ctx, component.UniformName.c_str());
                mu_layout_end_column(gui->Ctx);
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
            comp.Data[0] = value;
        } else if (type == "s2") {
            float x, y;
            iss >> x >> y;
            comp.Type = EGUIComponentTypeSlider2;
            comp.UniformName = uniformName;
            comp.Data[0] = x;
            comp.Data[1] = y;
        } else if (type == "s3") {
            float x, y, z;
            iss >> x >> y >> z;
            comp.Type = EGUIComponentTypeSlider3;
            comp.UniformName = uniformName;
            comp.Data[0] = x;
            comp.Data[1] = y;
            comp.Data[2] = z;
        } else if (type == "s4") {
            float x, y, z, w;
            iss >> x >> y >> z >> w;
            comp.Type = EGUIComponentTypeSlider4;
            comp.UniformName = uniformName;
            comp.Data[0] = x;
            comp.Data[1] = y;
            comp.Data[2] = z;
            comp.Data[3] = w;
        } else if (type == "c3") {
            float x, y, z;
            iss >> x >> y >> z;
            comp.Type = EGUIComponentTypeColor3;
            comp.UniformName = uniformName;
            comp.Data[0] = x;
            comp.Data[1] = y;
            comp.Data[2] = z;
        } else if (type == "c4") {
            float x, y, z, w;
            iss >> x >> y >> z >> w;
            comp.Type = EGUIComponentTypeColor4;
            comp.UniformName = uniformName;
            comp.Data[0] = x;
            comp.Data[1] = y;
            comp.Data[2] = z;
            comp.Data[3] = w;
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
                file << component.Data[0];
                file << std::endl;
                break;
            case EGUIComponentTypeSlider2:
                file << component.UniformName << " s2 ";
                file << component.Data[0] << " ";
                file << component.Data[1];
                file << std::endl;
                break;
            case EGUIComponentTypeSlider3:
                file << component.UniformName << " s3 ";
                file << component.Data[0] << " ";
                file << component.Data[1] << " ";
                file << component.Data[2];
                file << std::endl;
                break;
            case EGUIComponentTypeSlider4:
                file << component.UniformName << " s4 ";
                file << component.Data[0] << " ";
                file << component.Data[1] << " ";
                file << component.Data[2] << " ";
                file << component.Data[3];
                file << std::endl;
                break;
            case EGUIComponentTypeColor3:
                file << component.UniformName << " c3 ";
                file << component.Data[0] << " ";
                file << component.Data[1] << " ";
                file << component.Data[2];
                file << std::endl;
                break;
            case EGUIComponentTypeColor4:
                file << component.UniformName << " c4 ";
                file << component.Data[0] << " ";
                file << component.Data[1] << " ";
                file << component.Data[2] << " ";
                file << component.Data[3];
                file << std::endl;
                break;
        }
    }
    
    file.close();
    return true;
}

bool GUIComponentParse(uint32_t line_number, const std::string& gui_component_line, const std::string& uniform_line, const std::vector<GUIComponent>& previous_components, GUIComponent& out_component, std::string& out_parse_error) {
    auto ReportError = [&](const std::string& error) {
        out_parse_error = error + " at line " + std::to_string(line_number);
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
            memcpy(&out_component.Data, &previous_component.Data, sizeof(out_component.Data));
        }
    }
    return true;
}

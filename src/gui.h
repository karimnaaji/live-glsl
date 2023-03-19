#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <cstring>

#include <glad/gl.h>

struct GLFWwindow;

enum EGUIComponentType {
    EGUIComponentTypeSlider1,
    EGUIComponentTypeSlider2,
    EGUIComponentTypeSlider3,
    EGUIComponentTypeSlider4,
    EGUIComponentTypeColor3,
    EGUIComponentTypeColor4,
};

enum EGUIUniformType {
    EGUIUniformTypeFloat,
    EGUIUniformTypeVec2,
    EGUIUniformTypeVec3,
    EGUIUniformTypeVec4,
};

struct GUIComponentSliderRange {
    float Start;
    float End;
};

struct vec2 {
    float x,y;
};

struct vec3 {
    float x,y,z;
};

struct vec4 {
    float x,y,z,w;
};

struct GUIComponent {
    GUIComponent() {
        std::memset(this, 0x0, sizeof(GUIComponent));
    }
    GUIComponent(const GUIComponent& other) {
        Type = other.Type;
        UniformType = other.UniformType;
        UniformName = other.UniformName;
        IsInUse = other.IsInUse;
        SliderRange = other.SliderRange;
        memcpy(Data, other.Data, sizeof(Data));
    }
    EGUIComponentType Type;
    EGUIUniformType UniformType;
    std::string UniformName;
    bool IsInUse;
    union {
        GUIComponentSliderRange SliderRange;
    };
    float Data[4];
};

inline bool GUIIsSliderType(EGUIComponentType type) {
    return type == EGUIComponentTypeSlider1 ||
           type == EGUIComponentTypeSlider2 ||
           type == EGUIComponentTypeSlider3 ||
           type == EGUIComponentTypeSlider4;
}

inline uint32_t GUIComponents(EGUIComponentType type) {
    switch (type) {
        case EGUIComponentTypeSlider1:
        return 1;
        case EGUIComponentTypeSlider2:
        return 2;
        case EGUIComponentTypeColor3:
        case EGUIComponentTypeSlider3:
        return 3;
        case EGUIComponentTypeColor4:
        case EGUIComponentTypeSlider4:
        return 4;
    }
    assert(false);
    return 0;
}

inline uint32_t GUIUniformVariableComponents(EGUIUniformType uniform_type) {
    switch (uniform_type) {
        case EGUIUniformTypeFloat: return 1;
        case EGUIUniformTypeVec2: return 2;
        case EGUIUniformTypeVec3: return 3;
        case EGUIUniformTypeVec4: return 4;
    }
    assert(false);
    return 0;
}

struct GUITexture {
    GLuint Id;
    int Width;
    int Height;
};

typedef void* HGUI;

HGUI GUIInit(GLFWwindow* window_handle, int width, int height);
void GUIKeyCallback(HGUI gui, int key, int scancode, int action, int mods);
void GUIMouseButtonCallback(HGUI gui, int button, int action, int mods);
bool GUINewFrame(HGUI gui, std::vector<GUIComponent>& gui_components, std::vector<GUITexture> textures);
void GUIRender(HGUI gui);
void GUIResize(HGUI, int widht, int height);
void GUIDestroy(HGUI gui);
bool GUIComponentParse(uint32_t line_number, const std::string& gui_component_line, const std::string& uniform_line, const std::vector<GUIComponent>& previous_components, GUIComponent& out_component, std::string& out_parse_error);
bool GUIComponentSave(const std::string& path, const std::vector<GUIComponent>& components);
bool GUIComponentLoad(const std::string& path, std::vector<GUIComponent>& out_components);
void GUILog(HGUI handle, std::string log);
void GUIClearLog(HGUI handle);

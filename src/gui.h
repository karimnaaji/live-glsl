#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include "glm.hpp"

#define FONT_PATH1 "/Library/Fonts/Arial.ttf"
#define FONT_PATH2 "/Library/Fonts/Arial Unicode.ttf"

struct GLFWwindow;

enum EGUIComponentType {
    EGUIComponentTypeSlider1,
    EGUIComponentTypeSlider2,
    EGUIComponentTypeSlider3,
    EGUIComponentTypeSlider4,
    EGUIComponentTypeDrag1,
    EGUIComponentTypeDrag2,
    EGUIComponentTypeDrag3,
    EGUIComponentTypeDrag4,
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

struct GUIComponentDragRange {
    float Speed;
    float Start;
    float End;
};

struct GUIComponent {
    GUIComponent() {
        std::memset(this, 0x0, sizeof(GUIComponent));
    }
    GUIComponent(const GUIComponent& other) {
        std::memcpy(this, &other, sizeof(GUIComponent));
    }
    EGUIComponentType Type;
    EGUIUniformType UniformType;
    std::string UniformName;
    bool IsInUse;
    union {
        GUIComponentSliderRange SliderRange;
        GUIComponentDragRange DragRange;
    };
    union {
        float Vec1;
        glm::vec2 Vec2;
        glm::vec3 Vec3;
        glm::vec4 Vec4;
    };
};

inline bool GUIIsSliderType(EGUIComponentType type) {
    return type == EGUIComponentTypeSlider1 ||
           type == EGUIComponentTypeSlider2 ||
           type == EGUIComponentTypeSlider3 ||
           type == EGUIComponentTypeSlider4;
}

inline bool GUIIsDragType(EGUIComponentType type) {
    return type == EGUIComponentTypeDrag1 ||
           type == EGUIComponentTypeDrag2 ||
           type == EGUIComponentTypeDrag3 ||
           type == EGUIComponentTypeDrag4;
}

inline uint32_t GUIComponents(EGUIComponentType type) {
    switch (type) {
        case EGUIComponentTypeDrag1:
        case EGUIComponentTypeSlider1:
        return 1;
        case EGUIComponentTypeDrag2:
        case EGUIComponentTypeSlider2:
        return 2;
        case EGUIComponentTypeColor3:
        case EGUIComponentTypeDrag3:
        case EGUIComponentTypeSlider3:
        return 3;
        case EGUIComponentTypeColor4:
        case EGUIComponentTypeDrag4:
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

void GUIInit(GLFWwindow* window_handle);
bool GUINewFrame(std::vector<GUIComponent>& gui_components);
void GUIRender();
void GUIDestroy();
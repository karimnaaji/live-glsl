#pragma once

#include <vector>
#include <string>
#include <memory>
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

void GUIInit(GLFWwindow* window_handle);
bool GUINewFrame(std::vector<GUIComponent>& gui_components);
void GUIRender();
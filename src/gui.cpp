#include "gui.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_impl_glfw_gl3.cpp"

void GUIInit(GLFWwindow* window_handle) {
    ImGui_ImplGlfwGL3_Init(window_handle, true);

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowMinSize     = ImVec2(300, 5000);
    style.FramePadding      = ImVec2(6, 6);
    style.ItemSpacing       = ImVec2(6, 6);
    style.ItemInnerSpacing  = ImVec2(6, 6);
    style.Alpha             = 1.0f;
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 0.0f;
    style.IndentSpacing     = 6.0f;
    style.ItemInnerSpacing  = ImVec2(6, 6);
    style.ColumnsMinSpacing = 50.0f;
    style.GrabMinSize       = 14.0f;
    style.GrabRounding      = 0.0f;
    style.ScrollbarSize     = 12.0f;
    style.ScrollbarRounding = 0.0f;

    style.Colors[ImGuiCol_Text]                  = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.20f, 0.20f, 0.20f, 0.58f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 0.31f, 0.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.20f, 0.20f, 0.60f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.47f, 0.47f, 0.47f, 0.21f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.47f, 0.47f, 0.14f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.47f, 0.47f, 0.14f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Column]                = ImVec4(0.47f, 0.77f, 0.83f, 0.32f);
    style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.86f, 0.93f, 0.89f, 0.16f);
    style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.86f, 0.93f, 0.89f, 0.39f);
    style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.86f, 0.86f, 0.86f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
    style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.22f, 0.27f, 0.73f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);

    ImGuiIO& io = ImGui::GetIO();
    if (!io.Fonts->AddFontFromFileTTF(FONT_PATH1, 15)) {
        io.Fonts->AddFontFromFileTTF(FONT_PATH2, 15);
    }
}

bool GUINewFrame(std::vector<GUIComponent>& gui_components) {
    ImGui_ImplGlfwGL3_NewFrame();

    if (gui_components.size() == 0) return false;

    ImGuiWindowFlags options = ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Fixed Overlay", nullptr, ImVec2(0, 0), 0.3f, options);

    for (GUIComponent& component : gui_components) {
        switch (component.Type) {
            case EGUIComponentTypeSlider1:
            ImGui::SliderFloat(component.UniformName.c_str(),
                (float*)&component.Vec1,
                component.SliderRange.Start,
                component.SliderRange.End);
            break;
            case EGUIComponentTypeSlider2:
            ImGui::SliderFloat2(component.UniformName.c_str(),
                (float*)&component.Vec2,
                component.SliderRange.Start,
                component.SliderRange.End);
                break;
            case EGUIComponentTypeSlider3:
            ImGui::SliderFloat3(component.UniformName.c_str(),
                (float*)&component.Vec3,
                component.SliderRange.Start,
                component.SliderRange.End);
                break;
            case EGUIComponentTypeSlider4:
            ImGui::SliderFloat4(component.UniformName.c_str(),
                (float*)&component.Vec4,
                component.SliderRange.Start,
                component.SliderRange.End);
                break;
            case EGUIComponentTypeDrag1:
            ImGui::DragFloat(component.UniformName.c_str(),
                (float*)&component.Vec1,
                component.DragRange.Speed,
                component.DragRange.Start,
                component.DragRange.End);
                break;
            case EGUIComponentTypeDrag2:
            ImGui::DragFloat2(component.UniformName.c_str(),
                (float*)&component.Vec2,
                component.DragRange.Speed,
                component.DragRange.Start,
                component.DragRange.End);
                break;
            case EGUIComponentTypeDrag3:
            ImGui::DragFloat3(component.UniformName.c_str(),
                (float*)&component.Vec3,
                component.DragRange.Speed,
                component.DragRange.Start,
                component.DragRange.End);
                break;
            case EGUIComponentTypeDrag4:
            ImGui::DragFloat3(component.UniformName.c_str(),
                (float*)&component.Vec4,
                component.DragRange.Speed,
                component.DragRange.Start,
                component.DragRange.End);
                break;                
            case EGUIComponentTypeColor3:
            ImGui::ColorEdit3(component.UniformName.c_str(), (float*)&component.Vec3);
                break;
            case EGUIComponentTypeColor4:
            ImGui::ColorEdit4(component.UniformName.c_str(), (float*)&component.Vec4);
                break;
        }
    }

    return true;
}

void GUIRender() {
    ImGui::End();
    ImGui::Render();
}
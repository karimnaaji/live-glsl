#include "gui.h"

#include "utils.h"
#include "arial.ttf.h"

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
    ImFontConfig font_config;
    font_config.FontData = NULL;
    font_config.FontDataSize = 0;
    font_config.FontDataOwnedByAtlas = false;
    font_config.FontNo = 0;
    font_config.SizePixels = 0.0f;
    font_config.OversampleH = 3;
    font_config.OversampleV = 1;
    font_config.PixelSnapH = false;
    font_config.GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    font_config.GlyphRanges = NULL;
    font_config.MergeMode = false;
    font_config.MergeGlyphCenterV = false;
    font_config.DstFont = NULL;
    std::memset(font_config.Name, 0, sizeof(font_config.Name));

    ImFont* font = io.Fonts->AddFontFromMemoryTTF(arial_ttf, ARRAY_LENGTH(arial_ttf), 15, &font_config);
    assert(font != nullptr);
}

bool GUINewFrame(std::vector<GUIComponent>& gui_components) {
    ImGui_ImplGlfwGL3_NewFrame();

    if (gui_components.size() == 0) return false;

    uint32_t components_in_use = 0;
    for (const GUIComponent& component : gui_components) {
        if (component.IsInUse)
            ++components_in_use;
    }

    if (components_in_use == 0) return false;

    ImGuiWindowFlags options = ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Fixed Overlay", nullptr, ImVec2(0, 0), 0.3f, options);

    for (GUIComponent& component : gui_components) {
        if (!component.IsInUse) continue;
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
            ImGui::DragFloat4(component.UniformName.c_str(),
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

void GUIDestroy() {
    ImGui_ImplGlfwGL3_Shutdown();
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

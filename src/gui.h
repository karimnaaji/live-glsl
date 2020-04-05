#pragma once

#define FONT_PATH1 "/Library/Fonts/Arial.ttf"
#define FONT_PATH2 "/Library/Fonts/Arial Unicode.ttf"

struct GLFWwindow;

void GUIInit(GLFWwindow* window_handle);
void GUINewFrame();
void GUIRender();
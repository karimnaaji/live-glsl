#pragma once

static const GLchar* vertexShader = R"END( 
attribute vec4 position;

void main() {
    gl_Position = position;
}
)END";
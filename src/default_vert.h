#pragma once

static const GLchar* vertexShaderSrc = R"END( 
attribute vec4 position;

void main() {
    gl_Position = position;
}
)END";
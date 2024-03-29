#include "shader2.frag"

@pass(pass1, pass0, 512, 512)

uniform vec2 resolution;

out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    outColor = vec4(uv, 0.0, 1.0);
}

@pass_end
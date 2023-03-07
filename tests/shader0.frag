#include "shader1.frag"

@pass(main, pass1)

uniform vec2 resolution;

out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    outColor = vec4(uv, 0.0, 1.0);
}

@pass_end
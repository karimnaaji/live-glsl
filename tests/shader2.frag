@pass(pass0, 256, 256)

uniform vec2 resolution;

@path(heightmap_0.png)
uniform sampler2D heightmap;

out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    outColor = vec4(uv, 0.0, 1.0);
}

@pass_end
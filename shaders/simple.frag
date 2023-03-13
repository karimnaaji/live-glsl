uniform vec2 resolution;
uniform vec3 mouse;
uniform float time;

@slider1(0, 1)
uniform float p;

out vec4 color;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    color = vec4(uv, 0.0, 1.0);
}

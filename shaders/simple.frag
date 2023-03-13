precision mediump float;

uniform vec2 resolution;

out vec4 color;
void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    color = vec4(uv, 0.0, 1.0);
}
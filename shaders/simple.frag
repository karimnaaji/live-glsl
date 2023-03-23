uniform vec2 resolution;

out vec4 color;

@slider1(0.0, 1.0)
uniform float falloff;

float line_df(vec2 a, vec2 b, vec2 p) {
    vec2 ba = b - a;
    vec2 pa = p - a;
    float r = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - r * ba);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    float d = line_df(vec2(0.4), vec2(0.6), uv);
    color = vec4(vec3(pow(d, falloff)), 1.0);
}

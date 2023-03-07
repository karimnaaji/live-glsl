#define PI 3.14159265359

uniform vec2 resolution;
uniform float time;

out vec4 outColor;

float function(float x) {
    return fract(sin(time + x) * 4.0);
}

float lineJitter = 0.1;
float lineWidth = 7.0;
float gridWidth = 2.0;
float scale = 0.0005;
float zoom = 10.5;
vec2 offset = vec2(0.5);

float rand(float x) {
    return fract(sin(x) * 1e4);
}

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(rand(i), rand(i + 1.0), u);
}

float noise(vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

vec3 plot2D(vec2 uv, float width) {
    const float samples = 3.0;
    vec2 steping = width * vec2(scale) / samples;
    float count = 0.0;
    float mySamples = 0.0;
    for (float i = 0.0; i < samples; i++) {
        for (float j = 0.0;j < samples; j++) {
            if (i * i + j * j > samples * samples)
                continue;
            mySamples++;
            float ii = i + lineJitter * rand(vec2(uv.x + i * steping.x,uv.y + j * steping.y));
            float jj = j + lineJitter * rand(vec2(uv.y + i * steping.x,uv.x + j * steping.y));
            float f = function(uv.x + ii * steping.x) - (uv.y + jj * steping.y);
            count += (f > 0.0) ? 1.0 : -1.0;
        }
    }
    vec3 color = vec3(1.0);
    if (abs(count) != mySamples)
        color = vec3(abs(float(count)) / float(mySamples));
    return color;
}

vec3 grid2D(vec2 uv, float width) {
    float axisDetail = width*scale;
    if (abs(uv.x) < axisDetail || abs(uv.y) < axisDetail)
        return 1.0 - vec3(0.65, 0.15, 1.0);
    if (abs(mod(uv.x, 1.0)) < axisDetail || abs(mod(uv.y, 1.0)) < axisDetail)
        return 1.0 - vec3(0.80, 0.80, 1.0);
    if (abs(mod(uv.x, 0.25)) < axisDetail || abs(mod(uv.y, 0.25)) < axisDetail)
        return 1.0 - vec3(0.95, 0.95, 1.0);
    return vec3(0.0);
}

void main() {
    vec2 st = (gl_FragCoord.xy / resolution.xy) - offset;
    st.x *= resolution.x / resolution.y;

    scale *= zoom;
    st *= zoom;

    vec3 color = plot2D(st, lineWidth);
    color -= grid2D(st, gridWidth);

    outColor = vec4(color, 1.0);
}
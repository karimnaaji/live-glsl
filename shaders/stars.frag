
uniform vec2 resolution;
uniform float time;

@slider1(5, 100)
uniform float scale;
@slider1(1, 10)
uniform float intensity;
@slider1(0.1, 10)
uniform float flicker_speed;

out vec4 out_color;

float rand(vec2 v) {
    return 2.0 * fract(4356.17 * sin(1e4 * dot(v, vec2(1.0, 171.3)))) - 1.0;
}

#define MOD3 vec3(443.8975, 397.2973, 491.1871)
float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float flicker(float v) {
    return fract(sin(time * (flicker_speed / 1000.0) + v) * 342.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * resolution) / resolution.y * scale;

    vec2 gv = fract(uv) - 0.5;
    vec2 id = floor(uv);
    vec3 color = vec3(0.0);
    for (float i = -1.0; i <= 1.0; i++) {
        for (float j = -1.0; j <= 1.0; j++) {
            vec2 offset = vec2(i, j);
            float n = hash12(id + offset);
            vec2 uv_grid = gv - offset + vec2(n, fract(n * 34.0)) * 0.5;
            float d = length(uv_grid);
            float s = (intensity / 1000.0) / d * step(0.95, n) * n;
            s *= flicker(n) + 0.5;
            s *= smoothstep(1.0, 0.0, d);
            color += s;
        }
    }
    // if (gv.x > .48 || gv.y > .48) color.r = .5;

    out_color = vec4(color, 1.0);
}

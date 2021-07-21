#define PI 3.14159265359

uniform vec2 resolution;
uniform float time;

@color3
uniform vec3 color1;
@color3
uniform vec3 color2;
@color3
uniform vec3 color3;
@color3
uniform vec3 color4;

out vec4 outColor;

vec3 pal(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b * cos(6.28318 * (c * t + d));
}

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    vec3 col = pal( p.x, color1, color2, color3, color4);
    outColor = vec4( col, 1.0 );
}
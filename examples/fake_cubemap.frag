uniform vec2 resolution;
uniform vec3 mouse;
uniform float time;

out vec4 outColor;

float PI = 3.14159265359;

float random(vec3 p) {
    p = fract(p * vec3(23.2342, 97.1231, 91.2342));
    p += dot(p.zxy, p.yxz + 123.1234);
    return fract(p.x * p.y);
}

vec3 stars(vec3 p, float scale, vec2 offset) {
    vec2 uv_scale = (resolution / vec2(10.0)) * scale;
    vec3 position = vec3(p.xy * uv_scale + offset * resolution, p.z);

    vec3 q = fract(position) - 0.5;

    vec3 id = floor(position);

    float random_visibility = step(random(id), 0.01);
    float circle = smoothstep(0.5-0.1, 0.5+0.1, length(q));
    return (1.0 - vec3(circle)) * random_visibility;

    return vec3(circle * random_visibility);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy - 0.5;
    uv.x *= resolution.x / resolution.y;
    float aspect = resolution.x / resolution.y;
    float fov = PI * 0.6;
    float fov2 = fov / 2.0;

    vec2 m = mouse.xy / resolution.xy;

    m.y = 1.0 - m.y;

    float cx = cos(0.1 * time + 3.55);
    float sx = sin(0.1 * time + 3.55);
    float cy = 0.0;

    if(mouse.z > 0.) {
        cx = cos(10. * m.x);
        sx = sin(10. * m.x);
        cy = cos(3.2 * m.y);
    }

    vec3 cam_pos = vec3(cx - sx, cy, sx + cx);
    vec3 cam_dir = normalize(-cam_pos);

    vec3 ww = cam_dir;
    vec3 uu = normalize(cross(ww, vec3(0.0, 1.0, 0.0)));
    vec3 vv = normalize(cross(uu, ww));
    mat3 view_matrix = mat3(uu, vv, ww);

    vec3 dir = view_matrix * normalize(vec3(uv * tan(fov2), 1.0));
    vec3 n   = abs(dir);
    vec2 uv_remap = (n.x > n.y && n.x > n.z) ? dir.yz / dir.x:
                    (n.y > n.x && n.y > n.z) ? dir.zx / dir.y:
                                               dir.xy / dir.z;

    vec3 color = stars(vec3(uv_remap, 1.0), 1.0, vec2(0.0));

    // color = vec3(uv_remap, 1.0);
    outColor = vec4(color, 1.0);
}

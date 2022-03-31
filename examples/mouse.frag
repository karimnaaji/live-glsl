uniform vec2 resolution;
uniform vec3 mouse;
uniform float time;

out vec4 outColor;

void main() {
    if (mouse.z > 0.0) {
        vec2 normalized_mouse_coord = mouse.xy / resolution.xy;

        normalized_mouse_coord.y = 1.0 - normalized_mouse_coord.y;
        outColor = vec4(normalized_mouse_coord, 0.0, 1.0);
    } else {
        outColor = vec4(0.0);
    }
}
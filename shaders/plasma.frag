@pass(plasma, 512, 512)

uniform vec2 resolution;
uniform float time;

out vec4 outColor;

void main() {
    float tile_n = (int(gl_FragCoord.x) / 64) + ((int(gl_FragCoord.y) / 64) * 8);
    float tile_uvx = float(int(gl_FragCoord.x) % 64);
    float tile_uvy = float(int(gl_FragCoord.y) % 64);

    vec2 uv = vec2(tile_uvx, tile_uvy) / 64.0;

    float t = time * 0.2 + tile_n / 16.0;
    float v1 = sin(uv.x * 5.0 + t * 0.03);
    float v2 = sin(5.0 * (uv.x * sin(t * 0.02) + uv.y * cos(t * 0.33)) + t);
    float cx = uv.x + sin(t * 0.01) * 5.0;
    float cy = uv.y + sin(t * 0.053) * 5.0;
    float v3 = sin(sqrt(100.0 * (cx * cx + cy * cy)) + t * 0.01);
    float p = cos((v1 + v2 + v3) * 2.0) * 0.5 + 0.5;

    outColor = vec4(vec3(p), 1.0);
}

@pass_end

@pass(main, plasma)

uniform sampler2D plasma;
uniform vec2 resolution;
uniform float time;

@slider1(-2.0, 2.0)
uniform float camera_x;
@slider1(-2.0, 2.0)
uniform float camera_y;
@slider1(-2.0, 2.0)
uniform float camera_z;

out vec4 outColor;

float sdBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

vec3 raymarch(vec3 ro, vec3 rd) {
    float t = 0.0;
    vec3 box_size = vec3(0.5);
    for (int i = 0; i < 256; i++) {
        vec3 hit_point = ro + t * rd;
        float dist = sdBox(hit_point, box_size);
        if (dist < 0.0001) {
            vec3 half_box_size = box_size * 0.5;
            vec3 box_min = -half_box_size;
            vec3 uvw = ((hit_point - box_min) / box_size) * 0.5 + 0.25;
            return uvw;
        }
        t += dist;
    }
    return vec3(0.25, 0.25, 0.4);
}

vec3 texture3D(sampler2D sampler, vec3 uvw, float tile_per_row, float tile_size, float atlas_size) {
    float inv_tile_size = 1.0 / tile_per_row;
    float inv_atlas_size = 1.0 / atlas_size;

    uvw *= (tile_size - 1.0);

    float z0 = floor(uvw.z);
    float z1 = z0 + 1.0;
    float z_diff = uvw.z - z0;

    float x0 = mod(z0, tile_per_row) * tile_size;
    float x1 = mod(z1, tile_per_row) * tile_size;

    float y0 = floor(z0 * inv_tile_size) * tile_size;
    float y1 = floor(z1 * inv_tile_size) * tile_size;

    vec2 uv0 = vec2(x0 + uvw.x + 0.5, y0 + uvw.y + 0.5) * inv_atlas_size;
    vec2 uv1 = vec2(x1 + uvw.x + 0.5, y1 + uvw.y + 0.5) * inv_atlas_size;

    vec3 rgb0 = texture(sampler, uv0).ggg;
    vec3 rgb1 = texture(sampler, uv1).ggg;

    return mix(rgb0, rgb1, z_diff);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;

    uv = 2.0 * uv - 1.0;
    uv.x *= resolution.x / resolution.y;

    vec3 look_at = vec3(0.0);
    vec3 ray_origin = vec3(camera_x, camera_y, camera_z);

    vec3 forward = normalize(look_at - ray_origin);
    vec3 right   = normalize(cross(vec3(0, 1, 0), forward));
    vec3 up      = normalize(cross(forward, right));
    vec3 ray_direction = normalize(forward + uv.x * right + uv.y * up);

    vec3 uvw = raymarch(ray_origin, ray_direction);

    outColor = vec4(uvw, 1.0);
    outColor = vec4(texture3D(plasma, uvw, 8.0, 64.0, 512.0), 1.0);
}

@pass_end

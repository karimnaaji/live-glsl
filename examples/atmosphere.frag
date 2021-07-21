// Author: @karimnaaji

uniform vec2 resolution;
uniform float time;

out vec4 out_color;

precision highp float;

#define PI                      3.141592
#define BETA_R                  vec3(5.5e-6, 13.0e-6, 22.4e-6)
#define BETA_M                  vec3(21e-6, 21e-6, 21e-6)
#define MIE_G                   0.76
#define DENSITY_HEIGHT_SCALE_R  8000.0
#define DENSITY_HEIGHT_SCALE_M  1200.0
#define PLANET_RADIUS           6360e3
#define ATMOSPHERE_RADIUS       6420e3
#define SAMPLE_STEPS            10
#define DENSITY_STEPS           10

@slider1(0.0, 3.1415)
uniform float u_theta;
@slider1(0.0, 6.2830)
uniform float u_phi;
@slider1(0.0, 90.0)
uniform float u_fov;
@slider1(0.3, 50.0)
uniform float u_altitude;
@slider1(-3.14, 3.14)
uniform float u_rot_x;

vec3 ray_sphere_intersection(vec3 orig, vec3 dir, float radius) {
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, orig);
    float c = dot(orig, orig) - radius * radius;
    float d = sqrt(b * b - 4.0 * a * c);
    return vec3(vec2(-b - d, -b + d) / (2.0 * a), d);
}

vec3 extinction(vec2 density) {
    return exp(-vec3(BETA_R * density.x + BETA_M * density.y));
}

vec2 local_density(vec3 point) {
    float height = max(length(point) - PLANET_RADIUS, 0.0);
    return exp(-vec2(height / DENSITY_HEIGHT_SCALE_R, height / DENSITY_HEIGHT_SCALE_M));
}

float phase_ray(float cos_angle) {
    return (3.0 / (16.0 * PI)) * (1.0 + cos_angle * cos_angle);
}

float phase_mie(float cos_angle) {
    return (3.0 / (8.0 * PI)) * ((1.0 - MIE_G * MIE_G) * (1.0 + cos_angle * cos_angle)) /
        ((2.0 + MIE_G * MIE_G) * pow(1.0 + MIE_G * MIE_G - 2.0 * MIE_G * cos_angle, 1.5));
}

vec2 density_to_atmosphere(vec3 point, vec3 light_dir) {
    vec2 intersection = ray_sphere_intersection(point, light_dir, PLANET_RADIUS).xy;
    if (intersection.x > 0.0) {
        return vec2(1e20);
    }
    intersection = ray_sphere_intersection(point, light_dir, ATMOSPHERE_RADIUS).xy;
    float ray_len = intersection.y;
    float step_len = ray_len / float(DENSITY_STEPS);
    vec2 density_point_to_atmosphere = vec2(0.0);
    for (int i = 0; i < DENSITY_STEPS; ++i) {
        vec3 point_on_ray = point + light_dir * ((float(i) + 0.5) * step_len);
        density_point_to_atmosphere += local_density(point_on_ray) * step_len;;
    }

    return density_point_to_atmosphere;
}

vec3 atmosphere(vec3 ray_dir, vec3 ray_origin, vec3 sun_position, float sun_intensity) {
    vec2 density_orig_to_point = vec2(0.0);
    vec3 light_dir = normalize(sun_position);
    vec3 scatter_r = vec3(0.0);
    vec3 scatter_m = vec3(0.0);

    float ray_end = 1e20;
    vec3 intersection_planet = ray_sphere_intersection(ray_origin, ray_dir, PLANET_RADIUS);
    ray_end = intersection_planet.z < 0.0 ? 0.0 : min(intersection_planet.x, intersection_planet.y);

    float ray_start = 0.0;
    vec3 intersection_atmosphere = ray_sphere_intersection(ray_origin, ray_dir, ATMOSPHERE_RADIUS);
    ray_start = intersection_atmosphere.z < 0.0 ? 1e20 : min(intersection_atmosphere.x, intersection_atmosphere.y);

    if (intersection_atmosphere.z > 0.0 && intersection_planet.z < 0.0) {
        ray_end = max(intersection_atmosphere.x, intersection_atmosphere.y);
    }
    ray_origin += ray_dir * ray_start;
    float step_len = (ray_end - ray_start) / float(SAMPLE_STEPS);
    for (int i = 0; i < SAMPLE_STEPS; ++i) {
        vec3 point_on_ray = ray_origin + ray_dir * ((float(i) + 0.5) * step_len);

        // Local density
        vec2 density = local_density(point_on_ray) * step_len;
        density_orig_to_point += density;

        // Density from point to atmosphere
        vec2 density_point_to_atmosphere = density_to_atmosphere(point_on_ray, light_dir);

        // Scattering contribution
        vec2 density_orig_to_atmosphere = density_orig_to_point + density_point_to_atmosphere;
        vec3 extinction = extinction(density_orig_to_atmosphere);
        scatter_r += density.x * extinction;
        scatter_m += density.y * extinction;
    }

    // The mie and rayleigh phase functions describe how much light
    // is scattered towards the eye when colliding with particles
    float cos_angle = dot(ray_dir, light_dir);
    float phase_r = phase_ray(cos_angle);
    float phase_m = phase_mie(cos_angle);

    // Calculate light color
    vec3 beta_m = BETA_M;
    vec3 beta_r = BETA_R;
    vec3 out_color = (scatter_r * phase_r * beta_r + scatter_m * phase_m * beta_m) * sun_intensity;

    return out_color;
}

const float A = 0.15;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.30;

vec3 uncharted2_tonemap(vec3 x) {
   return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float rand(vec2 v) {
    return 2.0 * fract(4356.17 * sin(1e4 * dot(v, vec2(1.0, 171.3)))) - 1.0;
}

#define MOD3 vec3(443.8975,397.2973, 491.1871)
float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    float aspect = resolution.x / resolution.y;
    uv = (2.0 * uv - 1.0) * tan(u_fov / 2.0 * PI / 180.0);
    uv.x *= aspect;

    vec3 ray_dir = vec3(uv.x, uv.y, -1.0);
    vec3 color = vec3(0.0);
    vec3 sun_position = vec3(0.0, 0.2, -1.0);

    float a = u_rot_x;
    mat3 rot_x = mat3(
        1.0,    0.0,     0.0,
        0.0, cos(a), -sin(a),
        0.0, sin(a),  cos(a));
    ray_dir = rot_x * ray_dir;

    float altitude = mix(PLANET_RADIUS, ATMOSPHERE_RADIUS, u_altitude);
    vec3 position = vec3(
        0.0,
        altitude,
        0.0
    );

    color = atmosphere(normalize(ray_dir), position, normalize(sun_position), 25.0);

    float luminance = 5e-5;
    float white_scale = 1.1;
    color = uncharted2_tonemap((log2(2.0 / pow(luminance, 4.0))) * color) * white_scale;

    vec3 rnd = vec3(hash12(uv + fract(time)));
    color += rnd / 255.0;

    out_color = vec4(color, 1.0);
}

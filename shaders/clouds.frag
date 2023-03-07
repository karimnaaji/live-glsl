// Author: @karimnaaji

#define USE_GUI
#define DITHER
#define USE_TIME

uniform vec2 resolution;
#ifdef USE_TIME
uniform float time;
#else
const float time = 0.0;
#endif

out vec4 out_color;

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
#define CLOUD_STEPS             32


#ifdef USE_GUI

@slider1(-1.0, 1.0)
uniform float sun_y;
@slider1(0.0, 50.0)
uniform float sun_intensity;
@slider1(0.0, 10.0)
uniform float moon_intensity;
@slider1(0.0, 5.0)
uniform float density_scalar_m;
@slider1(0.0, 5.0)
uniform float density_scalar_r;
@color3
uniform vec3 rayleigh_color_tint;
@color3
uniform vec3 mie_color_tint;
@slider1(0.0, 1.0)
uniform float cloud_strength;
@slider1(0.0, 1.0)
uniform float cloud_noise_threshold;
@slider1(0.001, 0.1)
uniform float cloud_bias;
@slider1(0.0, 0.1)
uniform float cloud_speed;
@slider1(0.0, 1.0)
uniform float cloud_noise_seed;
@slider1(10.0, 10000.0)
uniform float cloud_start;
@slider1(10.0, 10000.0)
uniform float cloud_end;
@slider1(0.0, 0.5)
uniform float cloud_density_strength;


#define SUN_Y                   sun_y
#define SUN_INTENSITY           sun_intensity
#define MOON_INTENSITY          moon_intensity
#define DENSITY_SCALAR_M        density_scalar_m
#define DENSITY_SCALAR_R        density_scalar_r
#define COLOR_TINT_R            rayleigh_color_tint
#define COLOR_TINT_M            mie_color_tint
#define CLOUD_STRENGTH          cloud_strength
#define CLOUD_NOISE_THRESHOLD   cloud_noise_threshold
#define CLOUD_SPEED             cloud_speed
#define CLOUD_NOISE_SEED        cloud_noise_seed
#define CLOUD_START             cloud_start
#define CLOUD_END               cloud_end
#define CLOUD_BIAS              cloud_bias
#define CLOUD_DENSITY_STRENGTH  cloud_density_strength

#else

#define SUN_Y                   0.3
#define SUN_INTENSITY           20.0
#define MOON_INTENSITY          2.0
#define DENSITY_SCALAR_M        0.3
#define DENSITY_SCALAR_R        1.3
#define COLOR_TINT_R            vec3(1.164)
#define COLOR_TINT_M            vec3(0.9)
#define CLOUD_STRENGTH          0.6
#define CLOUD_NOISE_THRESHOLD   0.4
#define CLOUD_SPEED             0.015
#define CLOUD_NOISE_SEED        0.376
#define CLOUD_START             5150.0
#define CLOUD_END               8200.0
#define CLOUD_BIAS              0.02
#define CLOUD_DENSITY_STRENGTH  0.3

#endif

float mod289(float x) {return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x) {return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x) {return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p) {
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);
    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);
    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);
    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));
    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);
    return o4.y * d.y + o4.x * (1.0 - d.y);
}
float rand(float n){return fract(sin(n) * 43758.5453123);}

float noise(float p){
    float fl = floor(p);
  float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

#define NUM_OCTAVES 5
float fbm(vec3 x) {
    float v = 0.0;
    float a = 0.5;
    vec3 shift = vec3(100);
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(x);
        x = x * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

float fbm(float x) {
    float v = 0.0;
    float a = 0.5;
    float shift = float(100);
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(x);
        x = x * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

float ray_sphere_exit(vec3 orig, vec3 dir, float radius) {
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, orig);
    float c = dot(orig, orig) - radius * radius;
    float d = sqrt(b * b - 4.0 * a * c);
    return (-b + d) / (2.0 * a);
}

vec3 extinction(vec2 density) {
    return exp(-vec3(BETA_R * DENSITY_SCALAR_R * density.x + BETA_M * DENSITY_SCALAR_M * density.y));
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
    float ray_len = ray_sphere_exit(point, light_dir, ATMOSPHERE_RADIUS);
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

    float ray_len = ray_sphere_exit(ray_origin, ray_dir, ATMOSPHERE_RADIUS);
    float step_len = ray_len / float(SAMPLE_STEPS);
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
    vec3 beta_m = BETA_M * COLOR_TINT_M * DENSITY_SCALAR_M;
    vec3 beta_r = BETA_R * COLOR_TINT_R * DENSITY_SCALAR_R;
    vec3 out_color = (scatter_r * phase_r * beta_r + scatter_m * phase_m * beta_m) * sun_intensity;

    const float sun_angular_diameter = 0.9998;
    float sundisk = smoothstep(sun_angular_diameter, sun_angular_diameter + 0.00002, cos_angle);

    out_color = mix(out_color, vec3(sundisk), 0.5);

    return out_color;
}

vec3 clouds(vec3 ray_dir, vec3 position, vec3 sun_position) {
    float cloudscape_start = PLANET_RADIUS + CLOUD_START;
    float cloudscape_end = PLANET_RADIUS + CLOUD_END;
    float density = 0.0;
    float ray_start = ray_sphere_exit(position, ray_dir, cloudscape_start);
    float ray_end = ray_sphere_exit(position, ray_dir, cloudscape_end);
    float step_len = (ray_end - ray_start) / float(CLOUD_STEPS);
    vec3 ray_origin = ray_dir * ray_start;
    vec3 acc_color = vec3(0.0);
    vec3 light_dir = normalize(sun_position);
    for (int i = 0; i < CLOUD_STEPS; ++i) {
        vec3 point_on_ray = ray_origin + ray_dir * ((float(i) + 0.5) * step_len);
        float d = fbm(point_on_ray * CLOUD_NOISE_SEED / 1000.0 + time * CLOUD_SPEED);
        density += smoothstep(CLOUD_NOISE_THRESHOLD + CLOUD_BIAS, CLOUD_NOISE_THRESHOLD - CLOUD_BIAS, d);
    }
    density /= CLOUD_STEPS;
    density *= CLOUD_DENSITY_STRENGTH;
    return vec3(1.0 - exp(-density)) * smoothstep(1.0, 0.0, 1.0 - ray_dir.y);
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
    vec3 position = vec3(uv * 4.0 - 1.0, -1.0);
    position.y += 1.0;
    position.x -= 1.5;

    vec3 color = vec3(0.0);
    vec3 sun_position = vec3(0.0, SUN_Y, -1.0);

    vec3 sun_light = atmosphere(normalize(position), vec3(0.0, PLANET_RADIUS, 0), sun_position, SUN_INTENSITY);
    vec3 moon_light = vec3(0.0);
    if (sun_position.y < 0.0) {
        moon_light = atmosphere(normalize(position), vec3(0.0, PLANET_RADIUS, 0), vec3(0.0, 1.0, 0.0), MOON_INTENSITY);
        color = mix(sun_light, moon_light, -sun_position.y * 0.5 + 0.5);
    } else {
        color = sun_light;
    }

    color += clouds(normalize(position), vec3(0.0, PLANET_RADIUS, 0), sun_position) * CLOUD_STRENGTH;

    // Apply exposure
    float luminance = 5e-5;
    float white_scale = 1.0748724675633854;
    color = uncharted2_tonemap((log2(2.0 / pow(luminance, 4.0))) * color) * white_scale;

    // Dither
#ifdef DITHER
    vec3 rnd = vec3(hash12(uv + fract(time)));
    color += rnd / 255.0;
#endif

    out_color = vec4(color, 1.0);
}

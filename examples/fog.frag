//#define ANIMATE

uniform vec2 resolution;
uniform float time;

out vec4 out_color;

const mat2 m2 = mat2(1.6, -1.2, 1.2, 1.6);

float noi(vec2 p) {
    return 0.5 * (cos(6.2831 * p.x) + cos(6.2831 * p.y));
}

float terrain(vec2 p) {
    p *= 0.0013;
    float s = 1.;
    float t = 0.0;
    for (int i = 0; i < 3; i++) {
        t += s * noi(p);
        s *= 0.5 + 0.1 * t;
        p = 0.97 * m2 * p + (t - 0.5) * 0.2;
    }
    return t * 45.0;
}

vec2 map(vec3 pos) {
    float m = 0.0;
    float h = pos.y - terrain(pos.xz);
    float k = 60.0;
    float w = clamp(0.5 + 0.5 * (h) / k, 0.0, 1.0);
    h = h - k * w * (1.0 - w);
    m = mix(m, 1.0, w) - 1.0 * w * (1.0 - w);
    m = clamp(m,0.0,1.0);
    return vec2(h, m);
}

vec2 intersect(vec3 ro,vec3 rd,float tmin,float tmax ) {
    float t = tmin;
    float  m = 0.0;
    for(int i = 0; i < 100; i++) {
        vec3 pos = ro + t * rd;
        vec2 res = map(pos);
        m = res.y;
        if (res.x < (0.00001 * t) || t > tmax) break;
        t += res.x * 0.5;
    }
    return vec2(t, m);
}

mat3 set_camera(vec3 ro, vec3 ta, float cr) {
    vec3 cw = normalize(ta-ro);
    vec3 cp = vec3(sin(cr), cos(cr),0.0);
    vec3 cu = normalize(cross(cw,cp));
    vec3 cv = normalize(cross(cu,cw));
    return mat3(cu, cv, cw);
}

#define PI                      3.141592
#define BETA_R                  vec3(5.5e-6, 13.0e-6, 22.4e-6)
#define BETA_M                  vec3(21e-6, 21e-6, 21e-6)
#define MIE_G                   0.76
#define DENSITY_HEIGHT_SCALE_R  8000.0
#define DENSITY_HEIGHT_SCALE_M  1200.0
#define PLANET_RADIUS           6360e3
#define ATMOSPHERE_RADIUS       6420e3
#define SAMPLE_STEPS            16
#define DENSITY_STEPS           16

@slider1(0.0, 50.0)
uniform float sun_intensity;

@slider1(-1.0, 1.0)
uniform float light_x;
@slider1(-1.0, 1.0)
uniform float light_y;
@slider1(-1.0, 1.0)
uniform float light_z;
@slider1(0.0, 10.0)
uniform float moon_intensity;
@slider1(0.0, 5.0)
uniform float density_scalar_m;
@slider1(0.0, 1.0)
uniform float fog_depth_scale;
@slider1(0.0, 1.0)
uniform float sun_halo_intensity;
@slider1(0.0, 1.0)
uniform float fog_intensity;
@slider1(1.0, 10.0)
uniform float fog_depth_range;
@slider1(1.0, 10.0)
uniform float sun_halo_depth_range;
@color3
uniform vec3 fog_color;
@color3
uniform vec3 sun_halo_color;
@color3
uniform vec3 background_color;
@slider1(0.0, 5.0)
uniform float density_scalar_r;
@color3
uniform vec3 rayleigh_color_tint;

#define SUN_INTENSITY           sun_intensity
#define MOON_INTENSITY          moon_intensity
#define DENSITY_SCALAR_M        density_scalar_m
#define DENSITY_SCALAR_R        density_scalar_r
#define COLOR_TINT_R            rayleigh_color_tint

float ray_scphere_exit(vec3 orig, vec3 dir, float radius) {
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
    float ray_len = ray_scphere_exit(point, light_dir, ATMOSPHERE_RADIUS);
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

    float ray_len = ray_scphere_exit(ray_origin, ray_dir, ATMOSPHERE_RADIUS);
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
    vec3 beta_m = BETA_M;
    vec3 beta_r = BETA_R * COLOR_TINT_R;
    vec3 out_color = (scatter_r * phase_r * beta_r + scatter_m * phase_m * beta_m) * sun_intensity;

    const float sun_angular_diameter = 0.9998;
    float sundisk = smoothstep(sun_angular_diameter, sun_angular_diameter + 0.00002, cos_angle);

    out_color = mix(out_color, vec3(sundisk), 0.5);

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

void main() {
    vec2 xy = -1.0 + 2.0*gl_FragCoord.xy/resolution.xy;
    vec2 sp = xy*vec2(resolution.x/resolution.y,1.0);

    // camera setup
    float cr = 0.18 * sin(-0.1);
#ifdef ANIMATE
    vec3 ro = vec3(1000.0, 250.0, cos(time * .25) * 1000.0);
#else
    vec3 ro = vec3(1000.0, 250.0, 1000.0);
#endif

    vec3 ta = vec3(0.0, 0.0, 0.0);
    mat3 cam = set_camera(ro, ta, cr);

    // light
    vec3 light1 = normalize(vec3(light_x, light_y, light_z));

    // generate ray
    vec3 rd = cam * normalize(vec3(sp.xy, 1.0));

    // background
    vec3 bgcol = atmosphere(rd, vec3(0.0, PLANET_RADIUS, 0), light1, SUN_INTENSITY);

    // raymarch
    float tmin = 10.0;
    float tmax = 10000.0;

    float sundotc = clamp(dot(rd, light1), 0.0, 1.0);

    vec2 res = intersect(ro, rd, tmin, tmax);
    vec3 col = background_color;;

    // mountains
    float depth = res.x;

    float depth_range = fog_depth_range * 1e-7;
    float sun_depth_range = sun_halo_depth_range * 1e-8;

    vec3 halo = sun_halo_intensity * sun_halo_color;
    vec3 fog = fog_intensity * fog_color;

    // fog
    col = mix(col, mix(fog, halo, sundotc * sundotc * sun_halo_intensity), 1.0 - exp(-depth_range * depth * depth));

    // sun scatter
    float sun_halo = pow(sundotc, 16.0);
    col += halo * sun_halo * (1.0 - exp(-sun_depth_range * depth * depth));

    // background
    col = mix(col, bgcol, .5);

    // Apply exposure
    float luminance = 5e-5;
    float white_scale = 1.0748724675633854;
    col = uncharted2_tonemap((log2(2.0 / pow(luminance, 4.0))) * col) * white_scale;

    out_color = vec4(col, 1.0);
}
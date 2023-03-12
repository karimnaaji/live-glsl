#include "../../lygia/lighting/specular/ggx.glsl"

uniform vec2 resolution;

@path(heightmap_large.png)
uniform sampler2D tex0;
uniform vec2 tex0_resolution;
@path(heightmap_large_ao.png)
uniform sampler2D tex1;

@slider1(0, 360)
uniform float azimuth;

@slider1(0, 90)
uniform float altitude;
@slider1(0.1, 30)
uniform float shininess;
@slider1(0, 1)
uniform float roughness;
@slider1(0, 1)
uniform float F;
@slider1(0, 10)
uniform float diff_intensity;
@slider1(0, 10)
uniform float spec_intensity;
@slider1(0, 10)
uniform float ambient_intensity;
@slider1(0, 10)
uniform float ao_strength;
@slider1(0, 1)
uniform float shadow_strength;
@slider1(0.01, 1)
uniform float shadow_smoothness;
@slider1(0, 89)
uniform float shadow_angle;
@slider1(0, 500)
uniform float height_scale;

@color3
uniform vec3 spec_color;
@color3
uniform vec3 diff_color;
@color3
uniform vec3 ambient_color;
@color3
uniform vec3 color_0;
@color3
uniform vec3 color_1;
@color3
uniform vec3 color_2;
@color3
uniform vec3 color_3;

// #define PI (3.141592653589793)
#define RAD_TO_DEG (180.0 / PI)
#define DEG_TO_RAD (PI / 180.0)

out vec4 outColor;

float radians(float v) {
    return DEG_TO_RAD * v;
}

vec3 spherical_to_cartesian(float phi, float theta, float r) {
    vec3 v;
    v.x = r * sin(radians(phi)) * cos(radians(theta));
    v.y = r * sin(radians(phi)) * sin(radians(theta));
    v.z = r * cos(radians(phi));
    return v;
}

vec3 spherical_to_cartesian(float phi, float theta) {
    return spherical_to_cartesian(phi, theta, 1.0);
}

float diffuse_lambert(vec3 L, vec3 N) {
    float NdotL = dot(L, N);
    return max(0.0, NdotL);
}

float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 tonemap_rheinard(vec3 color) {
    float L = luminance(color);
    return color * (1.0 + color / (L + 0.01)) / (1.0 + color);
}

float height(vec2 uv) {
    return texture(tex0, uv).r;
}

vec3 normal(vec2 uv) {
    vec2 texel_size = 1.0 / tex0_resolution;

    float h01 = height(uv + vec2(-texel_size.x,         0.0));
    float h21 = height(uv + vec2( texel_size.x,         0.0));
    float h10 = height(uv + vec2(        0.0, -texel_size.y));
    float h12 = height(uv + vec2(        0.0,  texel_size.y));

    vec3 va = normalize(vec3(-1.0,  0.0, (h21 - h01) * height_scale));
    vec3 vb = normalize(vec3( 0.0, -1.0, (h12 - h10) * height_scale));

    return normalize(cross(va, vb));
}

float ao(vec2 uv) {
    return texture(tex1, uv).r;
}

float sample_shadow(vec2 uv, vec3 light, float sample_step) {
    float cos_shadow_angle = cos(radians(shadow_angle));
    float sin_shadow_angle = sin(radians(shadow_angle));
    vec2 texel_size = 1.0 / tex0_resolution;
    vec2 texel_dir = texel_size * sample_step * -normalize(light.xy);

    vec2 sample = uv;

    float marching_step = length(texel_dir) / cos_shadow_angle;

    float d = marching_step;
    float H = texture(tex0, uv).r;
    float horizon = 0.0;

    while (sample.x > 0.0 && sample.y > 0.0 && sample.x < 1.0 && sample.y < 1.0) {
        float sample_height = texture(tex0, sample).r;
        float light_dir_height = H + sin_shadow_angle * d;

        horizon = max(horizon, sample_height - light_dir_height);

        d += marching_step;
        sample += texel_dir;
    }

    return max(1.0 - smoothstep(0, 0.5 * shadow_smoothness, horizon), shadow_strength);
}


// specular occlusion term from the diffuse occlusion term (Lagarde)
// not have any physical basis but produces visually pleasant results.
float specular_ao(float NdotV, float ao, float roughness) {
    return clamp(pow(NdotV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / resolution.xy);
    vec3 N = normal(uv);
    vec3 L = spherical_to_cartesian(90.0 - altitude, azimuth);
    vec3 V = vec3(0.0, 0.0, 1.0);

    float NdotL = dot(L, N);
    float NdotV = dot(N, V);

    float ao = pow(ao(uv), ao_strength);
    float shadow = sample_shadow(uv, L, 3.0);

    vec3 diff = diff_intensity * diff_color * diffuse_lambert(L, N);
    vec3 spec = spec_intensity * spec_color * specularGGX(L, N, V, NdotV, NdotL, roughness, F) * specular_ao(NdotV, ao, roughness);
    vec3 ambient = ambient_intensity * ambient_color;
    vec3 color = tonemap_rheinard(vec3(spec + diff + ambient) * shadow);

    outColor = vec4(color * shadow, 1.0);
}

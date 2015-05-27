uniform float time;
uniform vec2 resolution;

vec4 sph1 = vec4(sin(time), 1.0, cos(time), 1.0);
vec3 l = vec3(0.5);

float sphere(in vec3 ro, in vec3 rd, in vec4 sph) {
    vec3 oc = ro - sph.xyz;

    float a = dot(rd, rd);
    float b = 2.0 * dot(oc, rd);
    float c = dot(oc, oc) - sph.w * sph.w;
    float delta = b * b - 4.0 * a * c;

    if(delta < 0.0) return -1.0;

    float t = (-b - sqrt(delta)) / 2.0 * a;
    return t;
}

vec3 nSphere(in vec3 pos, in vec4 sph) {
    return normalize(pos - sph.xyz);
}

float plane(in vec3 ro, in vec3 rd) {
    return -ro.y / rd.y;
}

float intersect(in vec3 ro, in vec3 rd, out float t) {
    t = 10000.0;
    float tsph = sphere(ro, rd, sph1);
    float tpla = plane(ro, rd);

    if(tsph > 0.0) {
        t = tsph;
        return 1.0;
    } 

    if(tpla > 0.0 && tpla < t) {
        t = tpla;
        return 2.0;
    }

    return -1.0;
}

void main(void) {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec3 ro = vec3(0.0, 3.0, 5.0);
    vec3 rd = normalize(vec3(-1.0 + 2.0 * uv, -1.0));

    float t = 0.0;
    float id = intersect(ro, rd, t);
    vec3 c = vec3(0.7);
    vec3 p = ro + t * rd;

    if (id == 1.0) { // sphere
        vec3 n = nSphere(p, sph1);
        float d = clamp(dot(n, l), 0.0, 1.0);
        c = vec3(0.3) + vec3(0.5) * d;
    } else if (id == 2.0) { // plane
        vec3 p = ro + t * rd;
        c = vec3(smoothstep(0.0, sph1.w, length(p.xz - sph1.xz)) * 0.7);
    }

    gl_FragColor = vec4(c, 1.0);
}

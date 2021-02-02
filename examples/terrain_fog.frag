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

    float s = 1.0;
    float t = 0.0;
    for(int i = 0; i<6; i++) {
        t += s * noi(p);
        s *= 0.5 + 0.1 * t;
        p = 0.97 * m2 * p + (t - 0.5) * 0.2;
    }

    return t * 55.0;
}

vec2 map(vec3 pos) {
    float m = 0.0;
    float h = pos.y - terrain(pos.xz);
    float k = 10.0;
    float w = clamp(0.5 + 0.5 * h / k, 0.0, 1.0 );
    m = mix(m, 1.0, w) - 1.0 * w * (1.0 - w);
    return vec2(h, m);
}

vec2 intersect(vec3 ro,vec3 rd,float tmin,float tmax ) {
    float t = tmin;
    float  m = 0.0;
    for(int i=0; i < 150; i++) {
        vec3 pos = ro + t*rd;
        vec2 res = map(pos);
        m = res.y;
        if (res.x < (0.001 * t) || t > tmax) break;
        t += res.x * 0.5;
    }
    return vec2(t, m);
}

vec3 dome(vec3 rd, vec3 light1) {
    float sda = clamp(0.5 + 0.5 * dot(rd, light1), 0.0, 1.0);
    float cho = max(rd.y, 0.0);
    vec3 bgcol = mix(mix(vec3(0.00, 0.10, 0.60) * 0.7,
                         vec3(0.50, 0.50, 0.20), pow(1.0 - cho, 3.0 + 4.0 - 4.0 * sda)),
                         vec3(0.43 + 0.2 * sda,0.4 - 0.1 * sda,0.4 - 0.25 * sda), pow(1.0 - cho, 10.0 + 8.0 - 8.0 * sda));
    return bgcol * 0.95;
}

mat3 setCamera(vec3 ro, vec3 ta, float cr) {
    vec3 cw = normalize(ta-ro);
    vec3 cp = vec3(sin(cr), cos(cr),0.0);
    vec3 cu = normalize(cross(cw,cp));
    vec3 cv = normalize(cross(cu,cw));
    return mat3(cu, cv, cw);
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
    mat3 cam = setCamera(ro, ta, cr);

    // light
    vec3 light1 = normalize(vec3(-0.8, 0.2, 0.5));

    // generate ray
    vec3 rd = cam * normalize(vec3(sp.xy, 1.5));

    // background
    vec3 bgcol = dome(rd, light1);

    // raymarch
    float tmin = 10.0;
    float tmax = 100000.0;

    float sundotc = clamp(dot(rd, light1), 0.0, 1.0);

    vec2 res = intersect(ro, rd, tmin, tmax);
    vec3  col = vec3(pow(res.y, 4.0)); // = bgcol;

    // mountains
    float t = res.x;

    // fog
    col = mix(col, 0.25 * mix(vec3(0.4, 0.75, 1.0), vec3(0.3,0.3,0.3), sundotc * sundotc), 1.0 - exp(-0.0000008*t*t));

    // sun scatter
    col += 0.55*vec3(0.5,0.3,0.3)*pow(sundotc, 8.0)*(1.0-exp(-0.003*t));

    // background
    col = mix(col, bgcol, 1.0 - exp(-0.00000004*t*t));

    // gamma
    col = pow(col, vec3(0.45));

    out_color = vec4(col, 1.0);
}
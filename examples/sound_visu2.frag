uniform float time;
uniform vec2 resolution;
uniform float spectrum[256];
uniform float wave[256];

//#define VISU_1
//#define VISU_2
#define PI 3.14159265

float line(vec2 p1, vec2 p2, vec2 p, float width, float spread)
{
    width = 1.0 / width;
    vec2 p2p1 = p1 - p2;
    vec2 p1p2 = -(p2p1);
    vec2 p2p = p - p2;
    vec2 p1p = p - p1;
    vec2 pp1 = -(p1p);
    vec2 pd = normalize(vec2(p2p1.y, -p2p1.x));
    float proj = dot(pd, pp1);
    float pr1 = dot(p2p1, p2p);
    float pr2 = dot(p1p2, p1p);

    if(pr1 > 0.0 && pr2 > 0.0) {
        return pow(1.0 / abs(proj * width), spread);    
    } else {    
        return 0.0;
    }
}

vec3 stripe(float p, float width, vec3 color1, vec3 color2) 
{
    float pmod = mod(p, width / 2.0);
    float w4 = width / 4.0;
    float f = abs(pmod-w4)/w4;
    float f1 = pow(f, 6.0);
    float f2 = pow(1.0 - f, 6.0);
    vec3 m1 = mix(color1, color2, f1);
    vec3 m2 = mix(color2, color1, f2);
    return pmod > w4 ? m1 : m2;
}

void main(void) 
{
    vec2 uv = vec2(gl_FragCoord.xy / resolution);
    vec2 p = -1.0 + 2.0 * uv;
    vec4 color = vec4(0.0);

#ifdef VISU_1
    float pad = PI / (256.0/10.0);
    int k = 0;
    for(float i = 0.0; i < 2.0 * PI; i+=pad) {
        if(spectrum[0] > 0.2) {
        float rad = 0.2 + (0.5 + 0.5 * wave[k++]);
        color.rgb += vec3(line(
            vec2(cos(i + time),sin(i + time)) * rad, 
            vec2(cos(i+pad + time), sin(i+pad + time)) * rad, 
            p, 0.005, 10.0));
        } 
    }
#else
#ifdef VISU_2
    for(int i = 0; i < 256; i+=10) {
        float x = 1.0/256.0 * float(i);
        color.g += line(vec2(x, 0.0), vec2(x, spectrum[i] * 2.0), uv, 0.005, 5.0);
    }
#else
    const vec3 col1 = vec3(0.2,0.25,0.5);
    const vec3 col2 = vec3(0.5,0.1,0.1);
    const float edgeSmooth = 0.015;
    float len = length(p);
    int ref1 = 2;
    int ref2 = 0;
    int ref3 = 10;

    if(spectrum[ref1] > 0.3) {
        float sphereRadius = spectrum[ref1];
        float a = 1.0 - smoothstep(sphereRadius, sphereRadius + edgeSmooth, len);
        color = vec4(vec3(a), 1.0);
    }

    float pad = PI / (1024.0/10.0);
    int k = 0;
    float offset = time + spectrum[2];
    for(float i = 0.0; i < 2.0 * PI; i+=pad) {
        if(spectrum[ref2] > 0.1) {
            float r = 0.7 + 0.5 * wave[k++];
            if(k > 256) k = 0;
            color.rgb += vec3(line(
                vec2(cos(i + offset),sin(i + offset)) * r + cos(i) * spectrum[ref2], 
                vec2(cos(i+pad + offset), sin(i+pad + offset)) * r + cos(i) * spectrum[ref2],  
                p, 0.005, 10.0));
        } 
    }

    color.rgb += mix(col1, col2, spectrum[ref2]);

    float width = spectrum[ref3];
    vec2 pmod = mod(p, width);
    color.rgb += stripe(vec2(pmod-(width/2.0)).y + sin(time * 0.5), 
        width, vec3(0.0), vec3(1.0)) * (width+0.7) * 0.01;
#endif
#endif

    gl_FragColor = color;
}

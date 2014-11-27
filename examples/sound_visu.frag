uniform float time;
uniform vec2 resolution;
uniform float spectrum[256];
uniform float wave[256];

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

void main(void) {
    vec2 uv = vec2(gl_FragCoord.xy / resolution);

    vec4 color = vec4(0.0);

    for(int i = 0; i < 256; i++) {
        float x = 1.0/256.0 * float(i);
        color.r += line(vec2(x, 0.0), vec2(x, wave[i]), uv, 0.005, 5.0);
        color.g += line(vec2(x, 0.0), vec2(x, spectrum[i]*4.0), uv, 0.005, 5.0);
    }

    gl_FragColor = color;
}
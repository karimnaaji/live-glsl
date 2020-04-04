uniform vec2 resolution;
uniform float time;

#define N 2.0 // oversampling (use power of 2)
#define rand(U) (2.0* fract(4356.17 * sin(1e4 * dot(U, vec2(1.0, 171.3)))) -1.0)

void main()
{
    vec2 R = resolution.xy;
    float z = 20.0;
    vec2 U = (2.0 * gl_FragCoord.xy - R) / R.y * z;
    vec2 Ur = U + 0.2 * rand(floor(U));

    vec4 O = vec4(0.0);
    for (float x = -1.0; x <= 1.0; x += 1.0 / N) {
        for (float y = -1.0; y <= 1.0; y += 1.0 / N) {
            vec2 Us = Ur + vec2(x, y) * z / R.y;
            float r = length(fract(Us) - 0.5 + floor(Us) - floor(U));
            O += 0.0005 / (pow(r / z, 3.0) * z * z);
        }
    }

    gl_FragColor = O * vec4(1.0, 0.5, 0.2, 1.0) / pow(N + N + 1.0, 2.0);
    gl_FragColor *= 0.00001 * rand(floor(U)) * (0.5 * cos(time * rand(floor(U))) + 0.5);
}

/*
 * Original shader from: https://www.shadertoy.com/view/llj3RD
 */

out vec4 outColor;

// glslsandbox uniforms
uniform float time;
uniform vec2 resolution;

// shadertoy emulation
#define iTime time
#define iResolution resolution
vec4 iMouse = vec4(0.);

// --------[ Original ShaderToy begins here ]---------- //
float snoise(vec3 uv, float res)
{
	const vec3 s = vec3(1e0, 1e2, 1e3);
	
	uv *= res;
	
	vec3 uv0 = floor(mod(uv, res))*s;
	vec3 uv1 = floor(mod(uv+vec3(1.), res))*s;
	
	vec3 f = fract(uv); f = f*f*(3.0-2.0*f);

	vec4 v = vec4(uv0.x+uv0.y+uv0.z, uv1.x+uv0.y+uv0.z,
		      	  uv0.x+uv1.y+uv0.z, uv1.x+uv1.y+uv0.z);

	vec4 r = fract(sin(v*1e-1)*1e3);
	float r0 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	r = fract(sin((v + uv1.z - uv0.z)*1e-1)*1e3);
	float r1 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	return mix(r0, r1, f.z)*2.-1.;
}

////////////////// distance field functions from http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}
/////////////////


// Utility stuff
#define PI 3.14159

mat3 rotx(float a) { mat3 rot; rot[0] = vec3(1.0, 0.0, 0.0); rot[1] = vec3(0.0, cos(a), -sin(a)); rot[2] = vec3(0.0, sin(a), cos(a)); return rot; }
mat3 roty(float a) { mat3 rot; rot[0] = vec3(cos(a), 0.0, sin(a)); rot[1] = vec3(0.0, 1.0, 0.0); rot[2] = vec3(-sin(a), 0.0, cos(a)); return rot; }

    

float toClipSpace(float f)
{
    return f * 2.0 - 1.0;
}


vec3 lookAt(vec3 from, vec3 to, vec3 dir)
{
    mat3 m;
    
    vec3 fwd = normalize(to - from);
    vec3 _up = vec3(0.0, 1.0, 0.0);
    vec3 r = cross(fwd, _up);
    vec3 u = cross(r, fwd);
    
    m[0] = r;
    m[1] = u;
    m[2] = fwd;
    vec3 d = m * dir;    
    d.z *= -1.0;
    return d;
}

mat3 rotation = mat3(0.);


// Density at any position in world
float worldDensity(vec3 position)
{
    float m = 9999.0;
    position = position * rotation;
    float s = sdSphere(position, 0.1);
    
    vec3 offset = vec3(0.0, 0.0, iTime * 0.010);
    
    float n = (snoise(position , 8.0)) * 0.08;
    n -= (snoise(position + offset, 20.0)) * 0.035;
    n += (snoise(position, 100.0)) * 0.010;
    
    if (s + n < 0.0)
    {
        return n;
    }
    
    return m;
}


// Ray marching parameters
const int STEPS = 50;
const float STEP = 0.005;    
const int STEPS_TO_SUN = 2;


// These variables can be tweaked
vec3 sun = vec3(0.0, 1.0, 0.0);
const float LIGHTNESS = 1.26;
const float ABSORPTION = 3.5;
const float INTENSITY = 1.75;
const float SUN_INTENSITY = 0.035;
const float SUN_SHARPNESS = 30.0;


vec4 basecolor = vec4(1.5, 0.5, 0.0, 0.0);
vec4 suncolor = vec4(1.0, 0.3, 0.0, 0.0);
vec2 uv = vec2(0.);

bool trace(vec3 from, vec3 dir, inout vec4 color)
{
    
    vec3 rp = from + dir * 0.85;
    vec4 cloudcolor = vec4(0.0, 0.0, 0.0, 0.0);
    bool hit = false;
    for (int i = 0; i < STEPS; ++i)
    {
        rp += dir * STEP;
        float densityAtPosition = worldDensity(rp);
        
        if (densityAtPosition <= 0.0 && cloudcolor.a <= 1.0)
        {
            // ambient color
            float density = abs(densityAtPosition) * ABSORPTION;
            cloudcolor.rgb += (1.0 - cloudcolor.a) * INTENSITY * pow(density, LIGHTNESS);
            cloudcolor.a += density;
            
            // in scattering from sun
            vec3 pos = rp;
            float d = 0.0;
            for (int j = 0; j < STEPS_TO_SUN; ++j)
            {

                pos += sun * STEP;
                float densityAtPosition = worldDensity(pos);
                d += abs(clamp(densityAtPosition, -1.0, 0.0)) * SUN_SHARPNESS;
            }
            d = 1.0 - clamp(d, 0.0, 1.0);
            cloudcolor += suncolor * d * SUN_INTENSITY;
            hit = true;
        }
        
        if(cloudcolor.a >= 1.0)
        {
           break;
        }
    }
    
    cloudcolor *= cloudcolor;
    float orange = smoothstep(0.7, 1.4, 1.3 - uv.y);

    float blue = 1.0 - orange;
    basecolor *= orange;
    basecolor += blue * vec4(0.0, 0.0, 0.2, 0.0);
    color = basecolor + cloudcolor;
    return hit;
}




void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    uv = vec2(fragCoord.x / iResolution.x, fragCoord.y / iResolution.x);
    uv.y += (iResolution.y / iResolution.x) * 0.4;
    
    vec2 mouse = iMouse.xy / iResolution.xy;
    mouse -= vec2(0.5);
    rotation = mat3(1);
    
    rotation *= roty(mouse.x * 10.0);
    rotation *= rotx(mouse.y * 10.0);
    
    vec3 camPos = vec3(0.0, 0.0, 1.0);    
    vec3 lk = vec3(-toClipSpace(uv.x), -toClipSpace(uv.y), -4.0);
    vec3 dir = lookAt(camPos, vec3(0.0), normalize(lk));
    vec4 color = vec4(0.0);
    
    trace(camPos, dir,  color);
    
    fragColor = color;
    
}

void main(void)
{
    mainImage(outColor, gl_FragCoord.xy);
    outColor.a = 1.;
}
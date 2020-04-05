// cf https://www.desmos.com/calculator/b4fa9jsmsq

#define gauss(x) exp(-.5*((x)*(x)) )
#define S(v)     smoothstep(1.5*fwidth(v),0.,abs(v))

#define fact(n)  1. // stub valid for 0,1. -> to implement: n! for n >= 2

// --- Bessel function https://en.wikipedia.org/wiki/Bessel_function
float J(float n, float x) { 
    float s = pow(x/2.,n) / fact(n), j = s;
    for (float p=1.; p<27.; p++)
        j += s *= - (x*x/4.) / (p*(n+p)) ;
    return j;
}
#define J1(x) J(1.,x)
// --- Airy intensity https://en.wikipedia.org/wiki/Airy_disk#Mathematical_formulation
float A(float x){ float v = 2.*J1(x)/x; return /* abs(x)>19.5 ? 0. : */ v*v; } 

// fast Airy intensity
//#define fA(x) ( abs(x) < 1.88 ? gauss(x/1.4) : ( gauss(x/1.4) + 2.7/abs(x*x*x) )/2. ) // enveloppe: 2.7/d^3
  #define fA(x) ( abs(x) < 1.88 ? gauss(x/1.4) : abs(x) > 6. ? 1.35/abs(x*x*x) : ( gauss(x/1.4) + 2.7/abs(x*x*x) )/2. ) // enveloppe: 2.7/d^3
 

void mainImage( out vec4 O, vec2 u )  // ---------------------------------------------
{
    vec2 R = iResolution.xy,
         U = 10.* ( 2.*u - R ) / R.y; U.y += 8.;
    float kC = 30.,                   // curves amplitude gain
          d = length(U);
    
    O = ( U.x<0. ? A(d) : fA(d) ) * vec4(1,.5,.25,0) * 10.; // Airy and fastAiry spots
    if ( U.y < - .4 && fract(U.y)<.5 ) O -= O;    // black mask, to emphase low values
    
    for (float y=0.; y<4.; y++)       // array of dots with 2^n intensity progression
        for (float x=0.; x< 5.; x++ ) 
            O += fA( 30.*length(U-vec2(5.+3.*x,4.+2.*pow(1.7,y))) ) * exp2(x+5.*y)/8. * vec4(.25,.5,1,0);
                  // 4.*R.y/20. // 1-pixel spot
    // --- curves
    O.r += S( kC*fA(U.x) - U.y );     // fast Airy function
    O.g += S( kC* A(U.x) - U.y );     // true Airy function
    O.b += S(U.y) + S(U.x)            // frame
        + S(U.x-2.1)*sin(10.*U.y);
    O = vec4( pow(O, vec4(1./2.2)) ); // to sRGB
    
}
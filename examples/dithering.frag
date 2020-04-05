uniform vec2 resolution;
uniform float time;

out vec4 color;

//#define ANIMATED
//#define CHROMATIC


//note: from https://www.shadertoy.com/view/4djSRW
// This set suits the coords of of 0-1.0 ranges..
#define MOD3 vec3(443.8975,397.2973, 491.1871)
float hash11(float p)
{
	vec3 p3  = fract(vec3(p) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
vec3 hash32(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

// ====
/*
//note: outputs x truncated to n levels
float trunc( float x, float n )
{
	return floor(x*n)/n;
}*/

//note: returns [-intensity;intensity[, magnitude of 2x intensity
//note: from "NEXT GENERATION POST PROCESSING IN CALL OF DUTY: ADVANCED WARFARE"
//      http://advances.realtimerendering.com/s2014/index.html
float InterleavedGradientNoise( vec2 uv )
{
    const vec3 magic = vec3( 0.06711056, 0.00583715, 52.9829189 );
    return fract( magic.z * fract( dot( uv, magic.xy ) ) );
}

#if 0
vec4 bluenoise( vec2 fc )
{
    return texture( iChannel2, fc / iChannelResolution[2].xy );
}
#endif

//note: works for structured patterns too
// [0;1[
float remap_noise_tri_erp( const float v )
{
    float r2 = 0.5 * v;
    float f1 = sqrt( r2 );
    float f2 = 1.0 - sqrt( r2 - 0.25 );
    return (v < 0.5) ? f1 : f2;
}

// note: valve edition
//       from http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// note: input in pixels (ie not normalized uv)
vec3 ScreenSpaceDither( vec2 vScreenPos )
{
	// Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR
	//vec3 vDither = vec3( dot( vec2( 171.0, 231.0 ), vScreenPos.xy + time ) );
    vec3 vDither = vec3( dot( vec2( 171.0, 231.0 ), vScreenPos.xy ) );
    vDither.rgb = fract( vDither.rgb / vec3( 103.0, 71.0, 97.0 ) );

    //note: apply triangular pdf
    //vDither.r = remap_noise_tri_erp(vDither.r)*2.0-0.5;
    //vDither.g = remap_noise_tri_erp(vDither.g)*2.0-0.5;
    //vDither.b = remap_noise_tri_erp(vDither.b)*2.0-0.5;

    return vDither.rgb / 255.0; //note: looks better without 0.375...

    //note: not sure why the 0.5-offset is there...
    //vDither.rgb = fract( vDither.rgb / vec3( 103.0, 71.0, 97.0 ) ) - vec3( 0.5, 0.5, 0.5 );
	//return (vDither.rgb / 255.0) * 0.375;
}


void main()
{
	vec2 pos = ( gl_FragCoord.xy / resolution.xy );

    const float c0 = 32.0;

    float its = mix( 0.0, 1.0 / 32.0, pos.x );

	vec3 outcol;

    if ( pos.y > 11.0/12.0 )
	{
		outcol = vec3(its);
	}
	else if ( pos.y > 10.0/12.0 )
	{
		outcol = vec3( its + 0.5 / 255.0 ); //rounded
	}
	else if ( pos.y > 9.0/12.0 )
	{
		//note: scanline dithering
		float ofs = floor(mod(gl_FragCoord.y,2.0))*0.5;
		outcol = vec3( its + ofs/255.0);
	}
	else if( pos.y > 8.0/12.0)
	{
        //note: offset r, g, b, limbo-style
		outcol = vec3(its, its + 1.0/3.0/256.0, its + 2.0/3.0/256.0);

		//note: "luminance" incr
		//float e = its - trunc( its, 255.0 ); // = fract( 255.0 * its ) / 255.0;
		//vec2 rg = mod( floor( vec2(4.0,2.0) * e * 255.0), 2.0 );
		//outcol = floor( its*255.0 )/255.0 + vec3(rg,0.0) / 255.0;
	}
    else if ( pos.y > 7.0/12.0 )
    {
        //note: 2x2 ordered dithering, ALU-based (omgthehorror)
		vec2 ij = floor(mod( gl_FragCoord.xy, vec2(2.0) ));
		float idx = ij.x + 2.0*ij.y;
		vec4 m = step( abs(vec4(idx)-vec4(0,1,2,3)), vec4(0.5) ) * vec4(0.75,0.25,0.00,0.50);
		float d = m.x+m.y+m.z+m.w;

		//alternative version, from https://www.shadertoy.com/view/MdXXzX
		//vec2 n = floor(abs( gl_FragCoord.xy ));
		//vec2 s = floor( fract( n / 2.0 ) * 2.0 );
		//float f = (  2.0 * s.x + s.y  ) / 4.0;
		//float rnd = (f - 0.375) * 1.0;
		//outcol = vec3(its + rnd/255.0 );

		outcol = vec3( its + d/255.0 );
    }
	else if ( pos.y > 6.0/12.0 )
	{
#if 0
		//note: 8x8 ordered dithering, texture-based
		const float MIPBIAS = -10.0;
		float ofs = texture( iChannel0, gl_FragCoord.xy/iChannelResolution[0].xy, MIPBIAS ).r;

		outcol = vec3( its + ofs/255.0 );
#endif
	}
    else if ( pos.y > 5.0/12.0 )
    {
        vec2 seed = gl_FragCoord.xy;
        #if defined( ANIMATED )
        seed += 1337.0*fract(time);
        #endif

        float rnd = InterleavedGradientNoise( seed );
        //rnd = remap_noise_tri_erp(rnd)*2.0-0.5; //note: terrible pattern

        #ifdef CHROMATIC
        outcol = its + vec3(rnd, 1.0-rnd, rnd)/255.0;
        #else
        outcol = its + vec3(rnd/255.0);
        #endif


    }
	else if( pos.y > 4.0/12.0 )
	{
        vec3 rnd = ScreenSpaceDither( gl_FragCoord.xy );
        #ifdef CHROMATIC
		outcol = vec3(its) + rnd;
        #else
        outcol = vec3(its) + rnd.g;
        #endif
	}
    else if( pos.y > 3.0/12.0 )
    {
        //note: from comment by CeeJayDK
		float dither_bit = 8.0; //Bit-depth of display. Normally 8 but some LCD monitors are 7 or even 6-bit.

		//Calculate grid position
		float grid_position = fract( dot( gl_FragCoord.xy - vec2(0.5,0.5) , vec2(1.0/16.0,10.0/36.0) + 0.25 ) );

		//Calculate how big the shift should be
		float dither_shift = (0.25) * (1.0 / (pow(2.0,dither_bit) - 1.0));

		//Shift the individual colors differently, thus making it even harder to see the dithering pattern
		#ifdef CHROMATIC
        vec3 dither_shift_RGB = vec3(dither_shift, -dither_shift, dither_shift); //subpixel dithering
        #else
        vec3 dither_shift_RGB = vec3(dither_shift, dither_shift, dither_shift); //non-chromatic dithering
        #endif

		//modify shift acording to grid position.
		dither_shift_RGB = mix(2.0 * dither_shift_RGB, -2.0 * dither_shift_RGB, grid_position); //shift acording to grid position.

		//shift the color by dither_shift
		outcol = its + 0.5/255.0 + dither_shift_RGB;
    }
    else if ( pos.y > 2.0/12.0 )
	{
#if 0
        vec2 seed = gl_FragCoord.xy;
        #if defined( ANIMATED )
        seed += 1337.0*fract(time);
        #endif

        #ifdef CHROMATIC
        vec3 bn = bluenoise(seed).rgb;
        vec3 bn_tri = vec3( remap_noise_tri_erp(bn.x),
                            remap_noise_tri_erp(bn.y),
                            remap_noise_tri_erp(bn.z) );
        outcol = vec3(its) + (bn_tri*2.0-0.5)/255.0;
        #else
        float bn = bluenoise(seed).r;
        float bn_tri = remap_noise_tri_erp(bn);
        outcol = vec3(its) + (bn_tri*2.0-0.5)/255.0;
        #endif
#endif
	}
    else if ( pos.y > 1.0/12.0 )
    {
        //note: triangluarly distributed noise, 1.5LSB
        vec2 seed = pos;
        #if defined( ANIMATED )
        seed += fract(time);
        #endif

        #ifdef CHROMATIC
		vec3 rnd = hash32( seed ) + hash32(seed + 0.59374) - 0.5;
        #else
        vec3 rnd = vec3(hash12( seed ) + hash12(seed + 0.59374) - 0.5 );
        #endif

		outcol = vec3(its) + rnd/255.0;
    }
	else
	{
        //note: uniform noise by 1 LSB
		//note: better to use separate rnd for rgb
        vec2 seed = pos;
        #if defined( ANIMATED )
        seed += fract(time);
        #endif

        #ifdef CHROMATIC
		vec3 rnd = hash32( seed );
        #else
        vec3 rnd = vec3( hash12( seed ) );
        #endif

		//note: texture-based
		//const float MIPBIAS = -10.0;
		//vec2 uv = gl_FragCoord.xy / iChannelResolution[1].xy + vec2(89,27)*fract(time);
		//float rnd = texture( iChannel1, uv, MIPBIAS ).r;

		outcol = vec3( its ) + rnd/255.0;
	}

	outcol.rgb = floor( outcol.rgb * 255.0 ) / 255.0;
    outcol.rgb *= c0;

    //note: black bars
    outcol -= step( mod(pos.y, 1.0/12.0), 0.0025);

	color = vec4( outcol, 1.0 );
}

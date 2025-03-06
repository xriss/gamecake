
#shader "waters"

/*

try feedback texture blends using extensions?

Just feed fbo textures in?

on desktop we may have

	GL_ARB_texture_barrier

on android we may have

	EXT_shader_framebuffer_fetch
	ARM_shader_framebuffer_fetch

*/

#version 300 es
#version 330
#ifdef VERSION_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

#include "gamecake_shader_funcs"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform mat4 inverse_modelview;
uniform mat4 inverse_projection;

uniform mat4 camera;
uniform mat4 incamera;

uniform vec4 time;

uniform vec4 offset; // center of water
uniform vec4 noise_add; // translate water noise by this
uniform vec4 noise_siz; // scale water noise by this

uniform sampler2D tex_last_color;
uniform sampler2D tex_last_depth;


#include "sky_function"

// a noise that works on lower precision hardware (tested, but...)
float goldienoise(vec3 p)
{
	p+=step(0.0,p)*46.0-23.0; // move away from origin to reduce obvious patern
	float r=fract(tan(distance(p.xy*1.61803398874989484820459,p.xy)*p.z)*p.x);
    return r!=r ? 0.0 : r; // replace nan with 0
}

// with 2d blend
float goldienoise(in vec3 p,float s)
{
	vec3 fs=p/s; fs.z=floor(fs.z);
	vec3 ff=fract(fs);
    vec4 fc=vec4(floor(fs.xy),ceil(fs.xy))*s;
    return mix(
    mix( goldienoise( vec3( fc.xy , fs.z ) ),
         goldienoise( vec3( fc.xw , fs.z ) ),ff.y),
    mix( goldienoise( vec3( fc.zy , fs.z ) ),
         goldienoise( vec3( fc.zw , fs.z ) ),ff.y),ff.x);
}

#define PI 3.1415926538
#define PI2 (2.0*PI)
#define MFB (1024.0)
#define MFA (0.0)

// a wormy wavy sort of noise using perlin waves
float wormnoise( vec2 uv , float scale , float lines,float t)
{
	vec2 uvs=uv/scale;
	vec4 uvf=vec4(fract(uvs),fract(uvs)-1.0);
	vec4 uvb=vec4(floor(uvs),ceil(uvs));

	vec4 b=vec4(
		goldienoise(vec3((uvb.xy),0.0)),
		goldienoise(vec3((uvb.zy),0.0)),
		goldienoise(vec3((uvb.xw),0.0)),
		goldienoise(vec3((uvb.zw),0.0))
	);

	vec4 s=vec4(
		goldienoise(vec3((uvb.xy),10.0)),
		goldienoise(vec3((uvb.zy),10.0)),
		goldienoise(vec3((uvb.xw),10.0)),
		goldienoise(vec3((uvb.zw),10.0))
	);

	b=cos( PI * (  (t+s) + ( cos(b*PI2)*uvf.xzxz + sin(b*PI2)*uvf.yyww )*lines*(s*0.5+0.5) ) );

	uvf*=uvf;
	uvf=sqrt(uvf.xzxz + uvf.yyww);
	uvf=b*pow(clamp(1.0-uvf,0.0,1.0),vec4(1.5));
	float f=uvf.x+uvf.y+uvf.z+uvf.w;

	return clamp(f,-1.0,1.0);

}

// calculate height
float hmap(vec2 v)
{
	// wormy wavy pattern
	float h=wormnoise(v+noise_add.xz,10.0,3.0,time[0]*0.5)*1.0;

	// flatten in the distance by using this height scalar
	float s=1.0 - smoothstep( MFA , MFB , length(v-offset.xz) );
	return (0.5+(h*s)*0.5);
}

// calculate normal
vec3 hmapn(vec2 v)
{
	float d=1.0/1024.0;

	vec3 p1=vec3( 0 , 0 , (hmap( vec2( v.x   , v.y   ) ))*2.0 );
	vec3 p2=vec3( d , 0 , (hmap( vec2( v.x+d , v.y   ) ))*2.0 );
	vec3 p3=vec3( 0 , d , (hmap( vec2( v.x   , v.y+d ) ))*2.0 );

	return normalize( cross(p2-p1,p3-p1) );
}

#ifdef VERTEX_SHADER

in vec3 a_vertex;

out vec2 xy;
out vec3 eye;

out vec4 pos;

void main()
{
	vec4 v=vec4(a_vertex.xyz + offset.xyz, 1.0);
	v.y+=hmap(v.xz);
	xy=v.xz;


#ifdef POSITION_CUSTOM
	POSITION_CUSTOM
#else
	gl_Position = projection * incamera * modelview * v;
#endif

	pos=gl_Position;

	eye=normalize( ( modelview * v ).xyz - camera[3].xyz ) ;

}

#endif
#ifdef FRAGMENT_SHADER

in vec4 pos;
in vec3 eye;

in vec2 xy;
out vec4 FragColor;

vec4 water(vec2 p,out vec3 r)
{

	float s=1.0 - smoothstep( 128.0 , 512.0 , length(p) );

	float h = 1.0-hmap(p);
	vec3 wn = hmapn(p).xzy;
	vec3 vn = normalize( mat3( modelview ) * wn );

	vec3 c1=SRGB(vec3( 0.0 , 0.0 , 0.4 ))*(0.25  + max(0.0,vn.y)*0.75 );
	vec3 c2=SRGB(vec3( 0.4 , 0.6 , 0.4 ))*( 0.5*h );
	vec4 c3=SRGB(vec4( 0.0 , 0.0 , 0.0 , 0.0 ));

	vec3 rn=normalize( reflect( eye , vn ) );
	r=rn;

	if( rn.y < 0.0 ) // only if reflecting up
	{
		c3=sky( rn , vec4( 1.0 , 1.0 , 0.0 , 0.0 ) ) * step( 0.0 , -rn.y ) ;
	}


	vec4 cc=vec4(c1+c2,0.125);
//	cc.a=length(cc)/4.0;
//	cc.rgb*=1.0/(cc.a*4.0);
	return mix(cc,c3,length(c3)) ;
}

vec3 PHRGB(vec4 c){return c.rgb*c.a*4.0;}

#ifdef NOFRAG
void main(void)
{
}
#else
void main(void)
{
	vec2 s=vec2(1.0/4.0);
	vec3 r;

#if defined(WATER_SOFT)
	vec3 c1=PHRGB( vec4( water(xy,r) )*SRGB(color) ); // water
	vec3 luv=vec3(0.5)+((pos.xyz/pos.w)*0.5);	// read this pixel from last color/depth
#ifdef DEPTH_RANGE_REVERSE
	float d=1.0-float(texture(tex_last_depth,luv.xy)); // scene depth
#else
	float d=float(texture(tex_last_depth,luv.xy)); // scene depth
#endif
	float t=1.0-clamp(dot(eye,r),0.0,1.0); // adjust transparency based on angle of eye to water
	float b=1.0-smoothstep( luv.z , 0.5+(((pos.z+4.0)/(pos.w+4.0))*0.5) , d ); // depth of water
	vec3 c2=HRGB( texture(tex_last_color,luv.xy+r.xy*r.zy*((1.0-b)*0.02)) ); // add a little offset using reflection scaled by water depth
	c1=mix(c1,vec3(0.5),smoothstep(15.0/16.0,1.0,b)); // white water at edges
	FragColor=RGBH(mix(c1,c2,  (0.10+b*0.40)*t ));

#else

	vec3 c1=PHRGB( vec4( water(xy,r) )*SRGB(color) );
	FragColor=RGBH(c1);

#endif

}
#endif

#endif


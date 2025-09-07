
#shader "zone_screen_draw_test"

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif

#include "gamecake_shader_funcs"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex0;
uniform sampler2D tex1;

#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
	v_texcoord=a_texcoord;

	gl_Position = vec4( a_vertex.xyz, 1.0 );
}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH) && defined(VERSION_ES)
precision highp float;
#endif

in vec2 v_texcoord;

out vec4 FragColor;

void main(void)
{
	float d = float( textureLod(tex0, v_texcoord, 0.0) ) ;

	vec3 c=vec3( fract(d*256.0) );

	c=clamp(c,0.0,1.0);
	FragColor=vec4( c , 1.0 );
}

#endif


#shader "zone_screen_bloom_draw"

#version 300 es
#version 330

#ifdef VERSION_ES
precision mediump float;
#endif


#shader "zone_screen_draw"

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif
  
#include "gamecake_shader_funcs"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2 v_texcoord;
out vec4 pos;

void main()
{
	v_texcoord=a_texcoord;

	gl_Position = vec4( a_vertex.xyz, 1.0 );
	pos=gl_Position;
}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH) && defined(VERSION_ES)
precision highp float;
#endif

in vec4 pos;

in vec2 v_texcoord;

out vec4 FragColor;

#ifdef DAYNIGHT
uniform vec4 day_night;
#endif

void main(void)
{
	vec4  c4=texture(tex0,v_texcoord);
	float d = 4.0*c4.a ; // may need to tweak this to change output brightness range
	vec3 m = SRGB(c4.rgb)*d ;
	vec4 s4 = texture(tex1, v_texcoord).rgba ;
	vec3 b = SRGB(texture(tex2, v_texcoord).rgb) ;
	float s=s4.a;
	
#ifdef COLOR_FIX
	m=COLOR_FIX;
#endif

#ifdef SHADOW_FIX
	s=SHADOW_FIX;
#endif

#ifdef BLOOM_FIX
	b=BLOOM_FIX;
#endif

#ifdef BLOOM_SCALE
	b*=float(BLOOM_SCALE);
#endif

	vec3 c;
	
#ifdef TWEAK

#if TWEAK==0

	c= m * s + b;

#elif TWEAK==1

	c= m ;

#elif TWEAK==2

	c=vec3( s,s,s );

#elif TWEAK==3

	c= m * s ;

#elif TWEAK==4

	c= b ;

#elif TWEAK==5

	c= m + b ;

#elif TWEAK==6

	c= s4.xyz ;

#endif

#else

	c= m * s + b ;

#endif

#ifdef FOG_COLOR
	
	vec3 luv=vec3(0.5)+((pos.xyz/pos.w)*0.5);	// read this pixel from last color/depth
	float depth=float(texture(tex3,luv.xy)); // scene depth

	const vec4 fog_color=vec4(FOG_COLOR);
	const vec4 fog_config=vec4(FOG_CONFIG);

#ifdef DEPTH_RANGE_REVERSE
	c=mix(c,fog_color.rgb,fog_color.a*smoothstep(-fog_config[0],-fog_config[1],-depth));
#else
	c=mix(c,fog_color.rgb,fog_color.a*smoothstep(fog_config[0],fog_config[1],depth));
#endif

#endif

#ifdef DAYNIGHT
	c=DAYNIGHT(c,day_night);
#endif

	c=clamp(c,0.0,1.0);

#ifdef GAMMA
	c=pow(c,vec3(1.0/float( GAMMA )));
#else
	c=RGBS(c);
#endif

	FragColor=vec4( c , 1.0 );

}

#endif

#shader "zone_screen_build_occlusion"

#define PI 3.1415926538
#define PI2 (2.0*PI)

#version 300 es
#version 330
#ifdef VERSION_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp sampler2D;
#else
precision mediump float;
precision mediump sampler2D;
#endif
#endif
  
uniform vec4 shadow_light; // normal of light, IE position of sun and power in W

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex;

uniform mat4 inverse_projection;


#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
	v_texcoord=a_texcoord;

	gl_Position = vec4( a_vertex.xyz, 1.0 );
}

#endif
#ifdef FRAGMENT_SHADER


in vec2 v_texcoord;

out vec4 FragColor;




// convert a uv into view space by sampling z buffer
vec3 depth_to_view(vec2 cc)
{
#ifdef DEPTH_RANGE_REVERSE
	vec4 t=vec4( cc , 1.0-texture(tex,cc).r , 1.0 );
#else
	vec4 t=vec4( cc , texture(tex,cc).r , 1.0 );
#endif
	vec4 p=( t*2.0 - 1.0 );

#ifdef POSITION_CUSTOM_UNDO
	POSITION_CUSTOM_UNDO
#endif

	p=inverse_projection * p;

	if(abs(p.w)<0.001){p.w=0.001;} // sanity
	p=p/p.w;
	return p.xyz;
}

// convert view space into depth space
vec3 view_to_depth(vec3 vv)
{
	vec4 p=projection * vec4( vv , 1.0);
	if(abs(p.w)<0.001){p.w=0.001;} // sanity
	return (p.xyz/p.w)*0.5 + 0.5;
}

// a noise that works on lower precision hardware (tested, but...)
float goldienoise(vec3 p)
{
	p=abs(p)+23.0; // move away from origin to reduce obvious patern
	float r=fract(tan(distance(p.xy*1.61803398874989484820459,p.xy)*p.z)*p.x);
    return r!=r ? 0.0 : r; // replace nan with 0
}

float random2d(vec2 st) {
    return goldienoise(vec3(st,1.0));
}

float pows(float s,float p)
{
	return s<0.0 ? -pow(-s,p) : pow(s,p) ;
}

vec3 reconstruct_normal( vec2 vv )
{
	vec2 oo_texel_size = vec2(1.0) / vec2( textureSize(tex,0) );
	vec3 p0=depth_to_view( vv );
	vec3 p1=depth_to_view( vv+ vec2(-oo_texel_size.x , 0.0 ) );
	vec3 p2=depth_to_view( vv+ vec2( oo_texel_size.x , 0.0 ) );
	vec3 p3=depth_to_view( vv+ vec2( 0.0             ,-oo_texel_size.y) );
	vec3 p4=depth_to_view( vv+ vec2( 0.0             , oo_texel_size.y) );
	
	vec3 c1,c2;	
	if( abs(p1.z-p0.z) < abs(p2.z-p0.z) ) { c1=p1-p0; } else { c1=p0-p2; }
	if( abs(p3.z-p0.z) < abs(p4.z-p0.z) ) { c2=p3-p0; } else { c2=p0-p4; }

	return normalize( cross( c2 , c1 ) ) ;
}

float ambient_occlusion( vec2 vv , vec3 nrm )
{
	if(float(texture(tex,vv))>=0.999) { return 0.5; } // disable when far far away
	
	vec2 texel_size = vec2( textureSize(tex,0) ); // size of screen in pixels
	vec2 aspect=vec2( texel_size.y/texel_size.x , 1.0 );
	vec2 vp=vv*texel_size; // each unit is a pixel
	vec2 hp=fract(vp/2.0)-0.5; // a hash unit start vector
	float ha=atan(hp.y,hp.x)+vv.x+vv.y;

	float slen=float(AO_SIZE);

	vec3 p1=depth_to_view( vv );
	vec3 p2=view_to_depth( p1+(vec3(slen,slen,0.0)) );
	if(p1.z!=p1.z) { return 0.5; } // give up
	float dlen=max(0.001,length(p2.xy-vv.xy)); // scale needed to adjust to depth

	float rots=PI2/(sqrt(float(AO_SAMPLES))*2.0);
	float dims=1.0/float(AO_SAMPLES);
	float ac=0.0;
	float dc=float(AO_SAMPLES);
	for(int ia=0;ia<int(AO_SAMPLES);ia++)
	{
		float fa=float(ia);
		float r=ha+fa*rots;
		vec2 cc=vec2(sin(r),cos(r))*aspect*dlen*(1.0-pow(fa*dims,2.0));
		vec3 ss=depth_to_view(cc+vv);
		float d=(p1.z-ss.z)/(slen*2.0);
		ac+=clamp(d,-2.0,2.0);//*smoothstep(-3.0,-1.0,-abs(d)) ;// 1.0-smoothstep( p1.z - slen , p1.z + slen , ss.z );
	}
	return smoothstep(-1.0,1.0,ac/dc);

}

#ifdef SHADOW

uniform mat4 camera;
uniform mat4 shadow_mtx;
uniform sampler2D shadow_map;

float shadow_occlusion( vec2 vv , vec3 nrm )
{

	vec2 texel_size = vec2( textureSize(tex,0) ); // size of screen in pixels
	vec2 vp=vv*texel_size; // each unit is a pixel
	vec2 hp=fract(vp/2.0)-0.5; // a hash unit start vector
	float ha=atan(hp.y,hp.x)+vv.x+vv.y;

	vec3 v=depth_to_view( vv );

	vec4 shadow_uv = shadow_mtx * camera * vec4(v,1.0) ;

	shadow_uv = (shadow_uv/shadow_uv.w) * 0.5 + 0.5;

	const vec4 shadow=vec4(SHADOW);

	float shadow_value = 0.0; // max( 0.0 , shadow_uv.w );

	if( (shadow_uv.x > 0.0)  && (shadow_uv.x < 1.0) && (shadow_uv.y > 0.0) && (shadow_uv.y < 1.0) && (shadow_uv.z > 0.0) && (shadow_uv.z < 1.0) )
	{
		vec3 sas=smoothstep( -1.00 , -0.80 , -abs( shadow_uv.xyz*2.0-1.0 )  );
		float fade=sas.x*sas.y*sas.z;

//		float shadow_min=1.0;
		float shadow_add=0.0;
		float shadow_tmp=0.0;
		vec2 shadow_texel_size = 2.0 / vec2( textureSize(shadow_map,0) );
		float rots=PI2/float(SHADOW_SAMPLES);
		float dims=0.5/float(SHADOW_SAMPLES);
		float ac=0.0;
		for(int ia=0;ia<int(SHADOW_SAMPLES);ia++)
		{
			float fa=float(ia);
			float r=ha+fa*rots;
			vec2 rr=vec2(sin(r),cos(r))*(1.0-fa*dims);
#ifdef DEPTH_RANGE_REVERSE
			shadow_tmp = 1.0-texture(shadow_map, shadow_uv.xy + rr*shadow_texel_size ).r ;
#else
			shadow_tmp = texture(shadow_map, shadow_uv.xy + rr*shadow_texel_size ).r ;
#endif
			shadow_add += shadow_tmp ;
//			shadow_min = min( shadow_min , shadow_tmp );
		}
		shadow_value = fade*max( shadow_value , smoothstep(	shadow[1] ,	shadow[2] ,
			shadow_uv.z - (shadow_add/float(SHADOW_SAMPLES))  ) );
			
	}
	return ( (1.0-shadow_value)*shadow[0] + (1.0-shadow[0]) ) ;
}
#endif

void main(void)
{
	vec3 nrm=reconstruct_normal(v_texcoord);

#ifdef SHADOW
	float s=shadow_occlusion(v_texcoord,nrm);
#else
	float s=1.0;
#endif
#ifdef SHADOW_SCALE
	s=clamp( (1.0-((1.0-s)*float(SHADOW_SCALE))) ,0.0,1.0);
#endif

	float t=ambient_occlusion(v_texcoord,nrm);

#ifdef AO_CLIP
	t=min(1.0,t/float(AO_CLIP));
#endif
#ifdef AO_SCALE
	t=clamp( (1.0-((1.0-t)*float(AO_SCALE))) ,0.0,1.0);
#endif
	
	FragColor=vec4( vec3(0.5)+(nrm.xyz*0.5) , t*s );

}

#endif




#shader "zone_screen_build_bloom_pick"

#define PI 3.1415926538
#define PI2 (2.0*PI)

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

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

uniform mat4 inverse_projection;


#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
	v_texcoord=a_texcoord;

	gl_Position = vec4( a_vertex.xyz, 1.0 );
}

#endif
#ifdef FRAGMENT_SHADER

in vec2 v_texcoord;

out vec4 FragColor;

void main(void)
{
//	vec4 m = SRGB(texture(tex0, v_texcoord).rgba) ;
//	FragColor=RGBS(vec4( (m.rgb + pow( m.rgb , vec3(4.0) ) ) * (4.0*m.a-1.0) , 1.0 ));

	vec3 c = HRGB(texture(tex0, v_texcoord).rgba) ;
	c=pow(c/vec3(2.0),vec3(2.0));
#ifdef BLOOM_FEEDBACK
	vec3 o = SRGB(texture(tex2, v_texcoord).rgb) ; // last frame
	FragColor=vec4(RGBS(mix(o,c,1.0/float(BLOOM_FEEDBACK))),1.0);
#else
	FragColor=vec4(RGBS(c),1.0);
#endif
}

#endif


#shader "zone_screen_build_blur"

#define PI 3.1415926538
#define PI2 (2.0*PI)

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

uniform sampler2D tex;

uniform mat4 inverse_projection;

// direction of blur
uniform vec2 blurd;

#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
	v_texcoord=a_texcoord;

	gl_Position = vec4( a_vertex.xyz, 1.0 );
}

#endif
#ifdef FRAGMENT_SHADER


in vec2 v_texcoord;

out vec4 FragColor;

void main(void)
{
	vec2 tc=v_texcoord;

	vec2 siz = blurd / vec2( textureSize(tex,0) );

	vec4  c;

#if BLUR == -1

	c= min( min(
		SRGB(texture(tex,tc             )) ,
		SRGB(texture(tex,tc+siz.xy*  1.0)) ) ,
		SRGB(texture(tex,tc+siz.xy* -1.0)) ) ;

#elif BLUR == 3

	c =SRGB(texture(tex,tc).rgba)*(1.0/3.0);
	c+=SRGB(texture(tex,tc+siz.xy* 1.0).rgba)*(1.0/3.0);
	c+=SRGB(texture(tex,tc+siz.xy*-1.0).rgba)*(1.0/3.0);

#elif BLUR == 5

	c =SRGB(texture(tex,tc).rgba)*(1.0/5.0);
	c+=SRGB(texture(tex,tc+siz.xy* 1.0).rgba)*(1.0/5.0);
	c+=SRGB(texture(tex,tc+siz.xy*-1.0).rgba)*(1.0/5.0);
	c+=SRGB(texture(tex,tc+siz.xy* 2.0).rgba)*(1.0/5.0);
	c+=SRGB(texture(tex,tc+siz.xy*-2.0).rgba)*(1.0/5.0);

#elif BLUR == 6

	c =SRGB(texture(tex,tc).rgba)*(2.0/6.0);
	c+=SRGB(texture(tex,tc+siz.xy* 1.0).rgba)*(1.0/6.0);
	c+=SRGB(texture(tex,tc+siz.xy*-1.0).rgba)*(1.0/6.0);
	c+=SRGB(texture(tex,tc+siz.xy* 2.0).rgba)*(1.0/6.0);
	c+=SRGB(texture(tex,tc+siz.xy*-2.0).rgba)*(1.0/6.0);

#elif BLUR == 22

	c =SRGB(texture(tex,tc).rgba)*(8.0/22.0);
	c+=SRGB(texture(tex,tc+siz.xy* 1.0).rgba)*(4.0/22.0);
	c+=SRGB(texture(tex,tc+siz.xy*-1.0).rgba)*(4.0/22.0);
	c+=SRGB(texture(tex,tc+siz.xy* 2.0).rgba)*(2.0/22.0);
	c+=SRGB(texture(tex,tc+siz.xy*-2.0).rgba)*(2.0/22.0);
	c+=SRGB(texture(tex,tc+siz.xy* 3.0).rgba)*(1.0/22.0);
	c+=SRGB(texture(tex,tc+siz.xy*-3.0).rgba)*(1.0/22.0);

#elif BLUR == 16

	c =SRGB(texture(tex,tc).rgba)*(2.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy* 1.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy*-1.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy* 2.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy*-2.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy* 3.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy*-3.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy* 4.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy*-4.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy* 5.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy*-5.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy* 6.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy*-6.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy* 7.0).rgba)*(1.0/16.0);
	c+=SRGB(texture(tex,tc+siz.xy*-7.0).rgba)*(1.0/16.0);

#elif BLUR_COUNT == 382

	c =SRGB(texture(tex,tc).rgba)*(128.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy* 1.0).rgba)*(64.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy*-1.0).rgba)*(64.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy* 2.0).rgba)*(32.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy*-2.0).rgba)*(32.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy* 3.0).rgba)*(16.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy*-3.0).rgba)*(16.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy* 4.0).rgba)*(8.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy*-4.0).rgba)*(8.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy* 5.0).rgba)*(4.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy*-5.0).rgba)*(4.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy* 6.0).rgba)*(2.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy*-6.0).rgba)*(2.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy* 7.0).rgba)*(1.0/382.0);
	c+=SRGB(texture(tex,tc+siz.xy*-7.0).rgba)*(1.0/382.0);

#endif

	FragColor=RGBS(c.rgba);

}

#endif


#shader "zone_screen_build_dark"

#define PI 3.1415926538
#define PI2 (2.0*PI)

#version 300 es
#version 330
#ifdef VERSION_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif
  
uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex;

uniform mat4 inverse_projection;


#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
	v_texcoord=a_texcoord;

	gl_Position = vec4( a_vertex.xyz, 1.0 );
}

#endif
#ifdef FRAGMENT_SHADER

in vec2 v_texcoord;

out vec4 FragColor;

void main(void)
{
	vec2 tc=v_texcoord;

	vec3 siz = vec3( 1.0 / vec2( textureSize(tex,0) ) , 0.0 );

	vec4  c;

#if DARK == 1

	vec4 b=texture(tex,tc             );

	c= min( min( min( min(
		b ,
		texture(tex,tc+siz.xz*  1.0) ) ,
		texture(tex,tc+siz.xz* -1.0) ) ,
		texture(tex,tc+siz.zy*  1.0) ) ,
		texture(tex,tc+siz.zy* -1.0) ) ;

	c=(c+c+c+b)/4.0;

#elif DARK == 2

	c= 	texture(tex,tc             ) +
		texture(tex,tc+siz.xz*  1.0) +
		texture(tex,tc+siz.xz* -1.0) +
		texture(tex,tc+siz.zy*  1.0) +
		texture(tex,tc+siz.zy* -1.0) +
		texture(tex,tc+siz.xz*  2.0) +
		texture(tex,tc+siz.xz* -2.0) +
		texture(tex,tc+siz.zy*  2.0) +
		texture(tex,tc+siz.zy* -2.0) ;

	c=c/9.0;

#endif

	FragColor=c.rgba;

}

#endif

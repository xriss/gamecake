
#header "gamecake_shader_funcs"

#if 0 // !defined(SRGBGAMMA)
vec4 SRGB(vec4 c){return c;}
vec3 SRGB(vec3 c){return c;}
vec4 RGBS(vec4 c){return c;}
vec3 RGBS(vec3 c){return c;}
vec3 HRGB(vec4 c){return c.rgb;}
vec4 RGBH(vec3 c){return vec4(c.rgb,0.25);}
#else
vec4 SRGB(vec4 c){return pow(c,vec4(2.2,2.2,2.2,1.0));}
vec3 SRGB(vec3 c){return pow(c,vec3(2.2));}
vec4 RGBS(vec4 c){return pow(c,vec4(0.45454545454,0.45454545454,0.45454545454,1.0));}
vec3 RGBS(vec3 c){return pow(c,vec3(0.45454545454));}
// fake HDR uses alpha as a floating point scalar in the range 0-4 with 8bit precision
vec3 HRGB(vec4 c){return pow(c.rgb,vec3(2.2))*c.a*4.0;}
vec4 RGBH(vec3 c){float a=max(1.0,max(max(c.r,c.g),c.b));return vec4(pow(c.rgb/a,vec3(0.45454545454)),a/4.0);}
#endif

/*

This is the default gamecake shader, which is controlled by a bunch of
defines to enable various features.

First we must describe what to do with xyz vertex attributes, only one
of SCR , RAW , XYZ and POS should be used at once.

	SCR

Screen space, no transforms applied.

	RAW

The raw transform mode, ignores the modelview and only applies the
projection. This is for vertexs pre-transformed into world coords and
is kinda just around for legacy reasons at this point.

	XYZ

The xyz transform mode, using modelview and projection, this is the default mode.

	CAM

The cam transform mode, using modelview and incamera and projection to
transform points. A replacement for XYZ if we need a little bit more
camera control. IE we are tweaking the view space.

	POS

Is a special 2d z mode where the z component is treated as 0 for view
transform but added to z in viewspace.

This allows very simple control over depth buffer sorting of 2d polys,
perfect for sprites etc.


	COLOR

Each vertex has a color attribute.

	NORMAL

Each vertex has a normal attribute.

	TEX

Each vertex has a UV attribute and we have a tex texture to bind to.

	TEX4

Each vertex has a UVWX attribute and we have a tex texture to bind to.


	PHONG

Phong style lighting with a shiny spot.

	LIGHT

Use a uniform normal to describe where the light is coming from and its
color, otherwise we assume white and down the camera.

	MATIDX

Use a lookup material for colors which are stored in a texture


*/

#header "gamecake_shader_head"

#if VERSION>120

#define IN in
#define OUT out

#else

#ifdef VERTEX_SHADER
#define IN attribute
#else
#define IN varying
#endif

#define OUT varying
#define texture texture2D
#define FragColor gl_FragColor

#endif

#ifdef POSITION_CUSTOM_UNIFORM
	POSITION_CUSTOM_UNIFORM
#endif


uniform mat4 camera;	// can be applied to modelview to remove the camera part of the transform
uniform mat4 incamera;	// inverse camera can be applied to modelview if camera is not yet applied
uniform mat4 modelview;
uniform mat4 projection;

uniform vec4 color;

#ifdef TEX
uniform sampler2D tex;
#endif
#ifdef TEX4
uniform sampler2D tex;
#endif

#ifdef MATIDX
uniform sampler2D tex_mat;
#endif

#ifdef BONE
uniform vec4 bones[BONE*3]; // 64 bones
uniform vec4 bone_fix; // min,max,0,0 (bone ids stored in bones array)
#endif

#ifdef LIGHT
uniform vec4  shadow_light;
uniform vec4  light_color;
#endif

#ifdef PHONG
uniform vec4  shadow_light;
#endif

#ifdef NTOON
uniform vec4  shadow_light;
#endif

#ifdef TEXNTOON
uniform vec4  shadow_light;
#endif

#header "gamecake_shader_vertex"

#ifdef VERTEX_SHADER

OUT vec4  v_color;

#ifdef TEX
OUT vec2  v_texcoord;
#endif
#ifdef TEX4
OUT vec4  v_texcoord;
#endif

#ifdef MATIDX
OUT float v_matidx;
#endif

#ifdef NORMAL
OUT vec3  v_normal;
#endif


IN vec3 a_vertex;

#ifdef COLOR
IN vec4 a_color;
#endif

#ifdef TEX
IN vec2 a_texcoord;
#endif
#ifdef TEX4
IN vec4 a_texcoord;
#endif

#ifdef MATIDX
IN float a_matidx;
#endif

#ifdef NORMAL
IN vec4  a_normal;
#endif

#ifdef BONE
IN vec4  a_bone;
#endif

#ifdef SHADOW
uniform mat4 shadow_mtx;
OUT vec4  shadow_uv;
#endif


#ifdef TEXBONE

IN vec4  a_bone;
uniform sampler2D fixbones;
uniform sampler2D texbones;
uniform float animframe;

#if VERSION>120

mat4 fixbone(int bidx,int frame)
{
	return transpose( mat4(
		texelFetch(fixbones,ivec2(bidx*3+0,frame),0),
		texelFetch(fixbones,ivec2(bidx*3+1,frame),0),
		texelFetch(fixbones,ivec2(bidx*3+2,frame),0),
		vec4(0.0,0.0,0.0,1.0)) );
}

mat4 texbone(int bidx,int frame)
{
	return transpose( mat4(
		texelFetch(texbones,ivec2(bidx*3+0,frame),0),
		texelFetch(texbones,ivec2(bidx*3+1,frame),0),
		texelFetch(texbones,ivec2(bidx*3+2,frame),0),
		vec4(0.0,0.0,0.0,1.0)) );
}


#else

mat4 fixbone(int bidx,int frame)
{
	return transpose( mat4(
		texture(fixbones,vec2(bidx*3+0,frame)),
		texture(fixbones,vec2(bidx*3+1,frame)),
		texture(fixbones,vec2(bidx*3+2,frame)),
		vec4(0.0,0.0,0.0,1.0)) );
}

mat4 texbone(int bidx,int frame)
{
	return transpose( mat4(
		texture(texbones,vec2(bidx*3+0,frame)),
		texture(texbones,vec2(bidx*3+1,frame)),
		texture(texbones,vec2(bidx*3+2,frame)),
		vec4(0.0,0.0,0.0,1.0)) );
}

#endif

mat4 getbone(int bidx)
{
	float fb=fract(animframe);
	float fa=1.0-fb;

	mat4 ma=texbone( bidx , int(animframe    ) );
	mat4 mb=texbone( bidx , int(animframe+1.0) );
	mat4 mab=(((fa*ma)+(fb*mb)));

	mat4 mc=fixbone( bidx , 0 );
	mat4 md=fixbone( bidx , 1 );
	mat4 me=fixbone( bidx , 2 );

//	mat4 mf=mat4( mat3(me)*mat3(mab) );
//	mf[3]=mab[3];
	mat4 mf=mat4(mat3(me)*mat3(mab)); // tweak rot/scale should apply localy
	mf[3].xyz=me[3].xyz+mab[3].xyz; // tweak pos should just be added in world space

	mat4 mr=md*mc*mf;

	return mr*mc;
}

#endif

#ifdef MAIN
void MAIN(void)
#else
void main(void)
#endif
{

#ifdef POINTSIZE
	gl_PointSize=POINTSIZE;
#endif


#ifdef POSITION_CUSTOM_V
	POSITION_CUSTOM_V
#else

#ifdef SCR
	vec4 v=vec4(a_vertex.xyz, 1.0);
#endif
#ifdef RAW
	vec4 v=vec4(a_vertex.xyz, 1.0);
#endif
#ifdef XYZ
	vec4 v=vec4(a_vertex.xyz, 1.0);
#endif
#ifdef CAM
	vec4 v=vec4(a_vertex.xyz, 1.0);
#endif
#ifdef POS
	vec4 v=vec4(a_vertex.xy, 0.0, 1.0);
#endif

#endif


#ifdef NORMAL
	vec3 n=vec3(a_normal.xyz);
#endif


#ifdef BONE

	mat4 m=mat4(0.0);
	if( a_bone[0] > 0.0 ) // got bones
	{
		for(int b=0;b<4;b++)
		{
			int i=int(a_bone[b])*3;
			if(i>=3)
			{
				m+=mat4(bones[i-3],bones[i-2],bones[i-1],vec4(0.0,0.0,0.0,1.0))
					*(1.0-fract(a_bone[b]));
			}
			else { break; }
		}

		v=v*m;

#ifdef NORMAL
		n=n*mat3(m);
#endif
	}

#endif


#ifdef TEXBONE

	if(a_bone[0]>0.0) //  some bone data
	{
		mat4 bm;
		mat4 bv;

		bm=getbone( int(a_bone[0]-1.0) );
		bv=(1.0-fract(a_bone[0]))*(bm);

		if(a_bone[1]>0.0)
		{
			bm=getbone( int(a_bone[1]-1.0) );
			bv+=(1.0-fract(a_bone[1]))*(bm);

			if(a_bone[2]>0.0)
			{
				bm=getbone( int(a_bone[2]-1.0) );
				bv+=(1.0-fract(a_bone[2]))*(bm);

				if(a_bone[3]>0.0)
				{
					bm=getbone( int(a_bone[3]-1.0) );
					bv+=(1.0-fract(a_bone[3]))*(bm);
				}
			}
		}


		v=bv*v;

#ifdef NORMAL
		n=mat3(bv)*n;
#endif

	}

#endif


#ifdef POSITION_CUSTOM
	POSITION_CUSTOM
#else

#ifdef SCR
	gl_Position = v;
#endif
#ifdef RAW
	gl_Position = projection * v;
#endif
#ifdef XYZ
	gl_Position = projection * modelview * v;
#endif
#ifdef CAM
	gl_Position = projection * incamera * modelview * v;
#endif
#ifdef POS
	gl_Position = projection * modelview * v;
	gl_Position.z+=a_vertex.z;
#endif

#endif


#ifdef SHADOW
	shadow_uv = ( shadow_mtx * camera * modelview * v ) ;
	shadow_uv = vec4( ( shadow_uv.xyz / shadow_uv.w ) * 0.5 + 0.5 ,
		normalize( mat3( shadow_mtx * camera * modelview ) * n ).z );
#endif

#ifdef NORMAL
	v_normal = normalize( mat3( camera * modelview ) * n );
#endif

#ifdef COLOR
	v_color=SRGB(a_color)*SRGB(color);
#else
	v_color=SRGB(color);
#endif

#ifdef TEX
	v_texcoord=a_texcoord;
#endif
#ifdef TEX4
	v_texcoord=a_texcoord;
#endif

#ifdef MATIDX
	v_matidx=a_matidx;
#endif

}

#endif



#header "gamecake_shader_fragment"

#ifdef FRAGMENT_SHADER

IN vec4  v_color;

#if VERSION>120
OUT vec4 FragColor;
#endif

#ifdef TEX
IN vec2  v_texcoord;
#endif
#ifdef TEX4
IN vec4  v_texcoord;
#endif

#ifdef MATIDX
IN float v_matidx;
#endif

#ifdef NORMAL
IN vec3  v_normal;
#endif

#ifdef SHADOW
uniform sampler2D shadow_map;
IN vec4  shadow_uv;
#endif


#ifdef MAIN
void MAIN(void)
#else
void main(void)
#endif
{

#ifdef TEXNTOON

	vec3 n=normalize(v_normal);
	vec3 s=shadow_light.xyz;
//	vec2 uv=clamp( v_texcoord + vec2( dot(n,s)*0.5 ,0.0) , vec2(0.0,0.0) , vec2(1.0,1.0) ) ;
	vec2 uv=v_texcoord + vec2( dot(n,s)*0.5 ,0.0) ;

	FragColor = SRGB(texture(tex, uv ).rgba) * v_color;

//	FragColor=vec4(n,1.0) * v_color;

#else

#ifdef TEX
	#ifdef TEXHAX

		if( v_texcoord[0] <= -1.0 ) // special uv request to ignore the texture (use -2 as flag) lets us mix textured and untextured polys
		{
			FragColor=v_color ;
		}
		else
		{
			FragColor=SRGB(texture(tex, v_texcoord)) * v_color ;
		}

	#else
		FragColor=SRGB(texture(tex, v_texcoord)) * v_color ;
	#endif
#else
	FragColor=v_color ;
#endif

#endif

#ifdef MATIDX
	float tex_mat_u=(v_matidx+0.5)/MATIDX;
	vec4 c1=SRGB(texture(tex_mat, vec2(tex_mat_u,0.25) ));
	vec4 c2=SRGB(texture(tex_mat, vec2(tex_mat_u,0.75) ));
#else
	vec4 c1=FragColor;
	vec4 c2=SRGB(vec4(1.0,1.0,1.0,16.0/255.0));
#endif

#ifdef NTOON

	vec3 n=normalize(v_normal);
	vec3 s=shadow_light.xyz;
	float l=dot(n,s);
	if(l>=0.0)
	{
		FragColor= vec4(  mix( c1.rgb , c1.rgb*(1.0+NTOON) , l ) , c1.a );
	}
	else
	{
		FragColor= vec4(  mix( c1.rgb*(1.0-NTOON) , c1.rgb , 1.0-l ) , c1.a );
	}

#endif

#ifdef LIGHT
	vec3 n=normalize( v_normal );
	vec3 s=shadow_light.xyz;
	float l=max( 0.0, dot(n,s)*shadow_light.w );

	FragColor= vec4(  c1.rgb *         max( l , 0.25 ) +
						(c2.rgb * pow( max( l , 0.0  ) , c2.a*255.0 )).rgb , c1.a );
#endif

#ifdef PHONG
	vec3 n=normalize(v_normal);
	vec3 s=shadow_light.xyz;
	float l=max( 0.0, dot(n,s)*shadow_light.w );
	FragColor= vec4(  c1.rgb *         max( l , 0.25 ) +
						(c2.rgb * pow( max( l , 0.0  ) , c2.a*255.0 )).rgb , c1.a );
#endif

#ifdef DISCARD
	if((FragColor.a)<DISCARD) discard;
#endif

#ifdef SHADOW

	const vec4 shadow=vec4(SHADOW);

	float shadow_value = max( 0.0 , shadow_uv.w );

	if( (shadow_uv.x > 0.0)  && (shadow_uv.x < 1.0) && (shadow_uv.y > 0.0) && (shadow_uv.y < 1.0) && (shadow_uv.z > 0.0) && (shadow_uv.z < 1.0) )
	{
		float shadow_tmp=0.0;
		float shadow_add=0.0;
		float shadow_min=1.0;
#if VERSION>120
		vec2 shadow_texel_size = 1.0 / vec2( textureSize(shadow_map,0) );
#else
		vec2 shadow_texel_size = 1.0 / vec2( 2048.0 );
#endif
		for(int x = -1; x <= 1; x++)
		{
			for(int y = -1; y <= 1; y++)
			{
				shadow_tmp = texture(shadow_map, shadow_uv.xy + vec2(x,y)*shadow_texel_size ).r ;
				shadow_add += shadow_tmp;
				shadow_min = min( shadow_min ,  shadow_tmp );
			}
		}
		shadow_value = max( shadow_value , smoothstep(	shadow[1] ,	shadow[2] ,
			shadow_uv.z - mix( shadow_min , shadow_add/9.0 , abs( shadow_uv.w ) ) ) );
	}
	FragColor=vec4( FragColor.rgb*( (1.0-shadow_value)*shadow[0] + (1.0-shadow[0]) ) , FragColor.a );

#endif

	FragColor=RGBS(FragColor);
}

#endif

#shader "gamecake_shader"

#version 300 es
#version 330
#version 100
#version 120
#ifdef VERSION_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif


#include "gamecake_shader_funcs"

#include "gamecake_shader_head"
#include "gamecake_shader_vertex"
#include "gamecake_shader_fragment"

#shader "pos_normal"
#define POS 1
#define NORMAL 1
#include "gamecake_shader"


#shader "pos_normal_light"
#define POS 1
#define NORMAL 1
#define LIGHT 1
#include "gamecake_shader"


#shader "pos_color"
#define POS 1
#define COLOR 1
#include "gamecake_shader"


#shader "pos_color_discard"
#define POS 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"


#shader "xyz_color"
#define XYZ 1
#define COLOR 1
#include "gamecake_shader"

#shader "xyz_color_discard"
#define XYZ 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "xyz_tex"
#define XYZ 1
#define TEX 1
#include "gamecake_shader"

#shader "xyz_tex4"
#define XYZ 1
#define TEX4 1
#include "gamecake_shader"

#shader "xyz_normal_tex_phong"
#define XYZ 1
#define NORMAL 1
#define TEX 1
#define PHONG 1
#include "gamecake_shader"

#shader "xyz_normal_tex4_phong"
#define XYZ 1
#define NORMAL 1
#define TEX4 1
#define PHONG 1
#include "gamecake_shader"

#shader "xyz_tex_discard"
#define XYZ 1
#define TEX 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "xyz_tex4_discard"
#define XYZ 1
#define TEX4 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "pos_tex"
#define POS 1
#define TEX 1
#include "gamecake_shader"

#shader "pos_tex4"
#define POS 1
#define TEX4 1
#include "gamecake_shader"

#shader "pos_tex_discard"
#define POS 1
#define TEX 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "pos_tex4_discard"
#define POS 1
#define TEX4 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "pos_tex_color"
#define POS 1
#define TEX 1
#define COLOR 1
#include "gamecake_shader"

#shader "pos_tex4_color"
#define POS 1
#define TEX4 1
#define COLOR 1
#include "gamecake_shader"


#shader "pos_tex_color_discard"
#define POS 1
#define TEX 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "pos_tex4_color_discard"
#define POS 1
#define TEX4 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "raw_color"
#define RAW 1
#define COLOR 1
#include "gamecake_shader"

#shader "raw_tex_color"
#define RAW 1
#define TEX 1
#define COLOR 1
#include "gamecake_shader"

#shader "raw_tex4_color"
#define RAW 1
#define TEX4 1
#define COLOR 1
#include "gamecake_shader"

#shader "raw_tex_color_discard"
#define RAW 1
#define TEX 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "raw_tex4_color_discard"
#define RAW 1
#define TEX4 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "raw_tex"
#define RAW 1
#define TEX 1
#include "gamecake_shader"

#shader "raw_tex4"
#define RAW 1
#define TEX4 1
#include "gamecake_shader"

#shader "raw_tex_discard"
#define RAW 1
#define TEX 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "raw_tex4_discard"
#define RAW 1
#define TEX4 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "xyz"
#define XYZ 1
#include "gamecake_shader"

#shader "xyz_discard"
#define XYZ 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "pos"
#define POS 1
#include "gamecake_shader"

#shader "pos_discard"
#define POS 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "xyz_mask"
#define XYZ 1
#define MASK 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "xyz_normal"
#define XYZ 1
#define NORMAL 1
#include "gamecake_shader"

#shader "xyz_normal_light"
#define XYZ 1
#define NORMAL 1
#define LIGHT 1
#include "gamecake_shader"

#shader "xyz_normal_mat"
#define XYZ 1
#define NORMAL 1
#define MATIDX 64.0
#define PHONG 1
#include "gamecake_shader"

#shader "xyz_normal_mat_bone"
#define XYZ 1
#define NORMAL 1
#define MATIDX 64.0
#define BONE 64
#define PHONG 1
#include "gamecake_shader"

#shader "xyz_normal_tex"
#define XYZ 1
#define NORMAL 1
#define TEX 1
#include "gamecake_shader"

#shader "xyz_normal_tex4"
#define XYZ 1
#define NORMAL 1
#define TEX4 1
#include "gamecake_shader"

#shader "xyz_normal_tex_ntoon"
#define XYZ 1
#define NORMAL 1
#define TEX 1
#define NTOON 0.75
#include "gamecake_shader"

#shader "xyz_normal_tex4_ntoon"
#define XYZ 1
#define NORMAL 1
#define TEX4 1
#define NTOON 0.75
#include "gamecake_shader"

#shader "xyz_normal_ntoon"
#define XYZ 1
#define NORMAL 1
#define NTOON 0.75
#include "gamecake_shader"


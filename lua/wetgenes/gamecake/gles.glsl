
	
/*

This is the default gamecake shader, which is controlled by a bunch of 
defines to enable various features.

First we must describe what to do with xyz vertex attributes, only one 
of USE_RAW , USE_XYZ and USE_POS should be used at once.

	USE_RAW

The raw transform mode, ignores the modelview and only applies the 
projection. This is for vertexs pre-transformed into world coords.

	USE_XYZ

The xyz transform mode, using modelview and projection, this is the default mode.

	USE_POS

Is a special 2d z mode where the z component is treated as 0 for view 
transform but added to z in viewspace.

This allows very simple control over depth buffer sorting of 2d polys, 
perfect for sprites etc.


	USE_COLOR

Each vertex has a color attribute.

	USE_NORMAL

Each vertex has a normal attribute.

	USE_TEX

Each vertex has a UV attribute and we have a tex texture to bind to.


	USE_PHONG

Phong style lighting with a shiny spot.

	USE_LIGHT

Use a uniform normal to describe where the light is coming from.

	USE_MAT

Use a lookup material for the phong colors.


*/

#header "gamecake_shader"

uniform mat4 modelview;
uniform mat4 projection;

uniform vec4 color;

varying vec4  v_color;

#ifdef USE_TEX
uniform sampler2D tex;
varying vec2  v_texcoord;
#endif

#ifdef USE_MAT
uniform vec4 colors[2*16]; // 16 materials
varying float v_matidx;
#endif

#ifdef USE_NORMAL
varying vec3  v_normal;
#endif

#ifdef USE_BONE
uniform vec4 bones[64*3]; // 64 bones
uniform vec4 bone_fix; // min,max,0,0 (bone ids stored in bones array)
#endif

#ifdef USE_LIGHT
uniform vec3  light_normal;
uniform vec4  light_color;
#endif

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;

#ifdef USE_COLOR
attribute vec4 a_color;
#endif

#ifdef USE_TEX
attribute vec2 a_texcoord;
#endif
 
#ifdef USE_MAT
attribute float a_matidx;
#endif

#ifdef USE_NORMAL
attribute vec4  a_normal;
#endif

#ifdef USE_BONE
attribute vec4  a_bone;
#endif

void main()
{

#ifdef USE_POINTSIZE
	gl_PointSize=USE_POINTSIZE;
#endif


#ifdef USE_POS
	vec4 v=vec4(a_vertex.xy, 0.0, 1.0);
#endif
#ifdef USE_RAW
	vec4 v=vec4(a_vertex.xyz, 1.0);
#endif
#ifdef USE_XYZ
	vec4 v=vec4(a_vertex.xyz, 1.0);
#endif

#ifdef USE_NORMAL
	vec3 n=vec3(a_normal.xyz);
#endif


#ifdef USE_BONE

	mat4 m=mat4(0.0);

	int b;
	if( a_bone[0] > 0.0 ) // got bones
	{
		for(b=0;b<4;b++)
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

#ifdef USE_NORMAL
		n=n*mat3(m);
#endif
	}

#endif


#ifdef USE_POS
	gl_Position = projection * modelview * v;
	gl_Position.z+=a_vertex.z;
#endif
#ifdef USE_RAW
	gl_Position = projection * v;
#endif
#ifdef USE_XYZ
	gl_Position = projection * modelview * v;
#endif

#ifdef USE_NORMAL
	v_normal = normalize( mat3( modelview ) * n );
#endif

#ifdef USE_COLOR
	v_color=a_color*color;
#else
	v_color=color;
#endif
	
#ifdef USE_TEX
	v_texcoord=a_texcoord;
#endif

#ifdef USE_MAT
	v_matidx=a_matidx;
#endif

}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

void main(void)
{

#ifdef USE_TEX
	if( v_texcoord[0] <= -1.0 ) // special uv request to ignore the texture (use -2 as flag)
	{
		gl_FragColor=v_color ;
	}
	else
	{
		gl_FragColor=texture2D(tex, v_texcoord) * v_color ;
	}
#else
	gl_FragColor=v_color ;
#endif

#ifdef USE_MAT
	int matidx=int(floor(v_matidx+0.5));
	vec4 c1=colors[0+matidx*2]*gl_FragColor;
	vec4 c2=colors[1+matidx*2];
#else
	vec4 c1=gl_FragColor;
	vec4 c2=vec4(1.0f,1.0f,1.0f,16.0f);
#endif

#ifdef USE_LIGHT
	vec3 n=normalize( v_normal );
	vec3 l=normalize( mat3( modelview ) * light_normal );
	
	gl_FragColor= vec4(  c1.rgb *      max( n.z      , 0.25 ) + 
						(c2.rgb * pow( max( dot(n,l) , 0.0  ) , c2.a )).rgb , c1.a );
#endif

#ifdef USE_PHONG
	vec3 n=normalize(v_normal);
	vec3 l=normalize(vec3(0.0,-0.5,1.0));
	gl_FragColor= vec4(  c1.rgb *      max( n.z      , 0.25 ) + 
						(c2.rgb * pow( max( dot(n,l) , 0.0  ) , c2.a )).rgb , c1.a );
#endif

#ifdef USE_DISCARD
	if((gl_FragColor.a)<0.25) discard;
#endif

}

#endif


#shader "gamecake_shader"
#include "gamecake_shader"

#shader "pos_normal"
#define USE_POS 1
#define USE_NORMAL 1
#include "gamecake_shader"


#shader "pos_normal_light"
#define USE_POS 1
#define USE_NORMAL 1
#define USE_LIGHT 1
#include "gamecake_shader"


#shader "pos_color"
#define USE_POS 1
#define USE_COLOR 1
#include "gamecake_shader"


#shader "pos_color_discard"
#define USE_POS 1
#define USE_COLOR 1
#define USE_DISCARD 1
#include "gamecake_shader"


#shader "xyz_tex"
#define USE_XYZ 1
#define USE_TEX 1
#include "gamecake_shader"


#shader "xyz_tex_discard"
#define USE_XYZ 1
#define USE_TEX 1
#define USE_DISCARD 1
#include "gamecake_shader"


#shader "pos_tex"
#define USE_POS 1
#define USE_TEX 1
#include "gamecake_shader"


#shader "pos_tex_discard"
#define USE_POS 1
#define USE_TEX 1
#define USE_DISCARD 1
#include "gamecake_shader"


#shader "pos_tex_color"
#define USE_POS 1
#define USE_TEX 1
#define USE_COLOR 1
#include "gamecake_shader"


#shader "pos_tex_color_discard"
#define USE_POS 1
#define USE_TEX 1
#define USE_COLOR 1
#define USE_DISCARD 1
#include "gamecake_shader"

#shader "raw_tex_color"
#define USE_RAW 1
#define USE_TEX 1
#define USE_COLOR 1
#include "gamecake_shader"

#shader "raw_tex_color_discard"
#define USE_RAW 1
#define USE_TEX 1
#define USE_COLOR 1
#define USE_DISCARD 1
#include "gamecake_shader"

#shader "raw_tex"
#define USE_RAW 1
#define USE_TEX 1
#include "gamecake_shader"

#shader "raw_tex_discard"
#define USE_RAW 1
#define USE_TEX 1
#define USE_DISCARD 1
#include "gamecake_shader"

#shader "xyz"
#define USE_XYZ 1
#include "gamecake_shader"

#shader "xyz_discard"
#define USE_XYZ 1
#define USE_DISCARD 1
#include "gamecake_shader"

#shader "pos"
#define USE_POS 1
#include "gamecake_shader"

#shader "pos_discard"
#define USE_POS 1
#define USE_DISCARD 1
#include "gamecake_shader"

#shader "xyz_mask"
#define USE_XYZ 1
#define USE_MASK 1
#define USE_DISCARD 1
#include "gamecake_shader"

#shader "xyz_normal"
#define USE_XYZ 1
#define USE_NORMAL 1
#include "gamecake_shader"

#shader "xyz_normal_light"
#define USE_XYZ 1
#define USE_NORMAL 1
#define USE_LIGHT 1
#include "gamecake_shader"

#shader "xyz_normal_mat"
#define USE_XYZ 1
#define USE_NORMAL 1
#define USE_MAT 1
#define USE_PHONG 1
#include "gamecake_shader"

#shader "xyz_normal_mat_bone"
#define USE_XYZ 1
#define USE_NORMAL 1
#define USE_MAT 1
#define USE_BONE 1
#define USE_PHONG 1
#include "gamecake_shader"




	
/*

This is the default gamecake shader, which is controlled by a bunch of 
defines to enable various features.

First we must describe what to do with xyz vertex attributes, only one 
of RAW , XYZ and POS should be used at once.

	RAW

The raw transform mode, ignores the modelview and only applies the 
projection. This is for vertexs pre-transformed into world coords and 
is kinda just around for legacy reasons at this point.

	XYZ

The xyz transform mode, using modelview and projection, this is the default mode.

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


	PHONG

Phong style lighting with a shiny spot.

	LIGHT

Use a uniform normal to describe where the light is coming from and its 
color, otherwise we assume white and down the camera.

	MATIDX

Use a lookup material for colors which are stored in a texture


*/

#header "gamecake_shader_head"

uniform mat4 modelview;
uniform mat4 projection;

uniform vec4 color;

varying vec4  v_color;

#ifdef TEX
uniform sampler2D tex;
varying vec2  v_texcoord;
#endif

#ifdef MATIDX
uniform sampler2D tex_mat;
varying float v_matidx;
#endif

#ifdef NORMAL
varying vec3  v_normal;
#endif

#ifdef BONE
uniform vec4 bones[64*3]; // 64 bones
uniform vec4 bone_fix; // min,max,0,0 (bone ids stored in bones array)
#endif

#ifdef LIGHT
uniform vec3  light_normal;
uniform vec4  light_color;
#endif



#header "gamecake_shader_vertex"

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;

#ifdef COLOR
attribute vec4 a_color;
#endif

#ifdef TEX
attribute vec2 a_texcoord;
#endif
 
#ifdef MATIDX
attribute float a_matidx;
#endif

#ifdef NORMAL
attribute vec4  a_normal;
#endif

#ifdef BONE
attribute vec4  a_bone;
#endif

void main()
{

#ifdef POINTSIZE
	gl_PointSize=POINTSIZE;
#endif


#ifdef POS
	vec4 v=vec4(a_vertex.xy, 0.0, 1.0);
#endif
#ifdef RAW
	vec4 v=vec4(a_vertex.xyz, 1.0);
#endif
#ifdef XYZ
	vec4 v=vec4(a_vertex.xyz, 1.0);
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


#ifdef POS
	gl_Position = projection * modelview * v;
	gl_Position.z+=a_vertex.z;
#endif
#ifdef RAW
	gl_Position = projection * v;
#endif
#ifdef XYZ
	gl_Position = projection * modelview * v;
#endif

#ifdef NORMAL
	v_normal = normalize( mat3( modelview ) * n );
#endif

#ifdef COLOR
	v_color=a_color*color;
#else
	v_color=color;
#endif
	
#ifdef TEX
	v_texcoord=a_texcoord;
#endif

#ifdef MATIDX
	v_matidx=a_matidx;
#endif

}

#endif



#header "gamecake_shader_fragment"

#ifdef FRAGMENT_SHADER

//#if defined(GL_FRAGMENT_PRECISION_HIGH)
//precision highp float; /* ask for better numbers if available */
//#endif

void main(void)
{

#ifdef TEX
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

#ifdef MATIDX
	float tex_mat_u=(v_matidx+0.5)/MATIDX;
	vec4 c1=texture2D(tex_mat, vec2(tex_mat_u,0.25) );
	vec4 c2=texture2D(tex_mat, vec2(tex_mat_u,0.75) );
#else
	vec4 c1=gl_FragColor;
	vec4 c2=vec4(1.0,1.0,1.0,16.0/255.0);
#endif

#ifdef LIGHT
	vec3 n=normalize( v_normal );
	vec3 l=normalize( mat3( modelview ) * light_normal );
	
	gl_FragColor= vec4(  c1.rgb *      max( n.z      , 0.25 ) + 
						(c2.rgb * pow( max( dot(n,l) , 0.0  ) , c2.a*255.0 )).rgb , c1.a );
#endif

#ifdef PHONG
	vec3 n=normalize(v_normal);
	vec3 l=normalize(vec3(0.0,-0.5,1.0));
	gl_FragColor= vec4(  c1.rgb *      max( n.z      , 0.25 ) + 
						(c2.rgb * pow( max( dot(n,l) , 0.0  ) , c2.a*255.0 )).rgb , c1.a );
#endif

#ifdef DISCARD
	if((gl_FragColor.a)<DISCARD) discard;
#endif

}

#endif

#header "gamecake_shader"
#include "gamecake_shader_head"
#include "gamecake_shader_vertex"
#include "gamecake_shader_fragment"

#shader "gamecake_shader"
#include "gamecake_shader"

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


#shader "xyz_tex"
#define XYZ 1
#define TEX 1
#include "gamecake_shader"


#shader "xyz_tex_discard"
#define XYZ 1
#define TEX 1
#define DISCARD 0.25
#include "gamecake_shader"


#shader "pos_tex"
#define POS 1
#define TEX 1
#include "gamecake_shader"


#shader "pos_tex_discard"
#define POS 1
#define TEX 1
#define DISCARD 0.25
#include "gamecake_shader"


#shader "pos_tex_color"
#define POS 1
#define TEX 1
#define COLOR 1
#include "gamecake_shader"


#shader "pos_tex_color_discard"
#define POS 1
#define TEX 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "raw_tex_color"
#define RAW 1
#define TEX 1
#define COLOR 1
#include "gamecake_shader"

#shader "raw_tex_color_discard"
#define RAW 1
#define TEX 1
#define COLOR 1
#define DISCARD 0.25
#include "gamecake_shader"

#shader "raw_tex"
#define RAW 1
#define TEX 1
#include "gamecake_shader"

#shader "raw_tex_discard"
#define RAW 1
#define TEX 1
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
#define BONE 1
#define PHONG 1
#include "gamecake_shader"



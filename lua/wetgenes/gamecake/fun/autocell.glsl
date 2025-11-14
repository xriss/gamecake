

#shader "fun_step_autocell"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif


uniform mat4 modelview;
uniform mat4 projection;
uniform sampler2D tex_cell;
uniform vec4  map_info; /* 2,3 the map texture size*/

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
	gl_Position = projection * vec4(a_vertex , 1.0);
	v_texcoord=a_texcoord;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

varying vec2  v_texcoord;


void main(void)
{
	vec4 c=texture2D(tex_cell, v_texcoord ).rgba;

	if( c.r==1.0 ) { c.r=0.0; } else { c.r+=1.0/255.0; }
	if( c.g==1.0 ) { c.g=0.0; } else { c.g+=1.0/255.0; }
	if( c.b==1.0 ) { c.b=0.0; } else { c.b+=1.0/255.0; }
	if( c.a==1.0 ) { c.a=0.0; } else { c.a+=1.0/255.0; }
	
//	c.r=4.0/255.0;
//	c.g=1.0/255.0;
//	c.b=31.0/255.0;
//	c.a=0.0/255.0;
	
	gl_FragColor=c;
}

#endif

#shader "fun_draw_autocell"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif


uniform mat4 modelview;
uniform mat4 projection;
uniform vec2 projection_zxy;
uniform vec4 color;
uniform sampler2D tex_cmap;
uniform sampler2D tex_tile;
uniform sampler2D tex_map;
uniform vec4  tile_info; /* 0,1 tile size eg 8x8 and 2,3 the font texture size*/
uniform vec4  map_info; /* 0,1 just add this to texcoord and 2,3 the map texture size*/

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4( a_vertex.xy + projection_zxy*a_vertex.z , 0.0 , 1.0);
    gl_Position.z+=a_vertex.z/16384.0;
	v_texcoord=a_texcoord;
	v_color=color;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

varying vec2  v_texcoord;
varying vec4  v_color;


void main(void)
{
	vec4 bg,fg; // colors
	vec4 c;
	vec4 d;
	vec2 uv=v_texcoord.xy+map_info.xy;		// base uv
	vec2 tc=fract(uv);						// tile uv
	vec2 tm=(floor(mod(uv,map_info.zw))+vec2(0.5,0.5))/map_info.zw;			// map uv
	
	d=texture2D(tex_map, tm).rgba;

//	d.r=4.0/255.0;
//	d.g=1.0/255.0;
//	d.b=31.0/255.0;
//	d.a=0.0/255.0;
	
	c=texture2D(tex_tile, (((d.rg*vec2(255.0,255.0))+tc)*tile_info.xy)/tile_info.zw ).rgba;
	fg=texture2D(tex_cmap, vec2( d.b,0.5) ).rgba;
	bg=texture2D(tex_cmap, vec2( d.a,0.5) ).rgba;

	c*=fg; // forground tint, can adjust its alpha
	c=((bg*(1.0-c.a))+c)* v_color; // background color mixed with pre-multiplied foreground and then finally tint all of it by the main color
 
	gl_FragColor=c;

}

#endif


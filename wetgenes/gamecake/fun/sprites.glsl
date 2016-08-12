

#shader "fun_draw_sprites"

#ifdef VERTEX_SHADER

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4(a_vertex, 1.0);
	v_texcoord=a_texcoord;
	v_color=color;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif


uniform sampler2D tex_char;
uniform sampler2D tex_map;

uniform vec4  char_info; /* 0,1 char size eg 8x8 and 2,3 the font texture size*/
uniform vec4  map_info; /* 0,1 just add this to texcoord and 2,3 the map texture size*/

varying vec2  v_texcoord;
varying vec4  v_color;


void main(void)
{
	vec4 c;
	vec4 d;
	vec2 uv=v_texcoord.xy+map_info.xy;		// base uv
	vec2 tc=fract(uv);						// char uv
	vec2 tm=(floor(uv)+vec2(0.5,0.5))/map_info.zw;			// map uv

	d=texture2D(tex_map, tm).rgba;	
	c=texture2D(tex_char, (((d.rg*vec2(255.0,255.0))+tc)*char_info.xy)/char_info.zw ).rgba;	

	gl_FragColor=c;

}

#endif


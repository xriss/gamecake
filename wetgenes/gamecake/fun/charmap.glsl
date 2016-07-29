

#shader "fun_draw_chars"

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


uniform sampler2D tex_fnt;
uniform sampler2D tex_map;
uniform sampler2D tex_pal;

uniform vec4  fnt_siz; /* 0,1 char size eg 8x8 and 2,3 the font texture size*/
uniform vec4  map_pos; /* 0,1 just add this to texcoord and 2,3 the map texture size*/

varying vec2  v_texcoord;
varying vec4  v_color;


void main(void)
{
	float c;
	vec4 d;
	vec2 t1=v_texcoord.xy+map_pos.xy;				// input uv
	vec2 t2=floor(mod(t1.xy,fnt_siz.xy)) ;			// char uv
	vec2 t3=floor(t1.xy/fnt_siz.xy)/map_pos.zw;		// map uv

	d=texture2D(tex_map, t3).rgba;	
	c=texture2D(tex_fnt, (t2+(floor((d.xy*255.0*fnt_siz.xy)+vec2(0.5,0.5))))/fnt_siz.zw , -16.0).r;
	gl_FragColor=c;

}

#endif


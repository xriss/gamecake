

#shader "fun_draw_canvas"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec2 projection_zxy;
uniform vec4 color;

uniform sampler2D tex_cmap;
uniform sampler2D tex_tile;
uniform sampler2D tex_canvas;

uniform vec4  canvas_info; /* 0,1 just add this to texcoord and 2,3 the map texture size*/

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4( a_vertex.xy + projection_zxy*a_vertex.z , 0.0 , 1.0);
    gl_Position.z+=a_vertex.z/65536.0;
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
	vec4 c;
	vec4 d;
	vec2 uv=v_texcoord.xy+canvas_info.xy;		// base uv
	vec2 tm=(floor(mod(uv,canvas_info.zw))+vec2(0.5,0.5))/canvas_info.zw;			// map uv
	
	d=texture2D(tex_canvas, tm).rgba;
	c=texture2D(tex_cmap, vec2( d.r,0.5) ).rgba;
 
	gl_FragColor=c*v_color;
	if((gl_FragColor.a)<0.0625) discard;

}

#endif


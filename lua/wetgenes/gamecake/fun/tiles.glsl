

#shader "fun_draw_tiles_debug"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex_cmap;
uniform sampler2D tex_tile;

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4(a_vertex.xy, 0.0 , 1.0);
    gl_Position.z+=a_vertex.z;
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

	gl_FragColor=texture2D(tex_tile, v_texcoord).rgba;

}

#endif


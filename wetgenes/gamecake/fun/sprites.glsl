

#shader "fun_draw_sprites"

#ifdef VERTEX_SHADER

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec4 a_color;
attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * vec4(a_vertex.xyz , 1.0);
	v_texcoord=a_texcoord;
	v_color=a_color;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif


uniform sampler2D tex;

varying vec2  v_texcoord;
varying vec4  v_color;

uniform vec4 color;

void main(void)
{
	gl_FragColor=texture2D(tex, v_texcoord).rgba*v_color*color;
	if((gl_FragColor.a)<0.0625) discard;
}

#endif


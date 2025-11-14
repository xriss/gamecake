

#shader "fun_draw_sprites"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec2 projection_zxy;
uniform vec3 modelview_add;
uniform vec4 color;
uniform sampler2D tex;

#ifdef VERTEX_SHADER

attribute vec4 a_color;
attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    vec3 pos=a_vertex+modelview_add;
    gl_Position = projection * modelview * vec4( pos.xy + floor(0.5+projection_zxy*pos.z) , 0.0 , 1.0);
    gl_Position.z+=pos.z/16384.0;
	v_texcoord=a_texcoord;
	v_color=a_color;
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
	gl_FragColor=texture2D(tex, v_texcoord).rgba*v_color*color;
	if((gl_FragColor.a)<0.0625) discard;
}

#endif


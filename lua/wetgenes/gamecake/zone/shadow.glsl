
#shader "zone_shadow_draw"

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif



#shader "zone_shadow_map"

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif



#shader "zone_shadow_test"

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
precision mediump sampler2D;
#endif
  
uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D shadow_map;


#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
	v_texcoord=a_texcoord;

	gl_Position = vec4( a_vertex.xyz, 1.0 );
}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH) && defined(VERSION_ES)
precision highp float;
#endif

in vec2 v_texcoord;

out vec4 FragColor;

void main(void)
{
	float t=texture(shadow_map, v_texcoord).r*256.0*2.0;

	vec3 rgb=0.5+sin(t+vec3( 0.0 , 2.09439510239 , 4.18879020479 ))*0.5;
	
	FragColor=vec4(rgb,1.0);
//	FragColor=texture(tex, v_texcoord);
}

#endif


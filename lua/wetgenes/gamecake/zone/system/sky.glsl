
#header "sky_function"

uniform vec4 sun;

#define PI 3.1415926538
#define PI2 (2.0*PI)
#define PIO2 (PI/2.0)


vec4 sky(vec3 eye, vec4 blend)
{
	float daynight=sin(sun[0]);

	vec3 color_sky=mix(     vec3( 0.0 , 0.2 , 0.7 ) , vec3( 0.0 , 0.0 , 0.1 ) , daynight );
	vec3 color_horizon=mix( vec3( 0.2 , 0.2 , 0.5 ) , vec3( 0.0 , 0.0 , 0.4 ) , daynight );
	vec3 color_floor=mix(   vec3( 0.0 , 0.0 , 0.0 ) , vec3( 0.0 , 0.0 , 0.0 ) , daynight );
	vec3 color_sun=vec3( 1.0 , 0.8 , 0.4 );

	vec3 color=color_horizon;
	color=mix( color , color_sky   , pow( smoothstep(0.0,1.0,-eye.y) , 1.0 ) );
	color=mix( color , color_floor , pow( smoothstep(0.0,0.1, eye.y) , 1.0 ) );

	vec2 ang=mod( vec2( asin(eye.x)+PI , atan(eye.y,eye.z)+PI-(sun[0]) ) , PI2 )-PI ;

    float d=( 1.0-smoothstep( sun[2] , sun[3] , length(ang) ) ) * smoothstep(-0.1,0.0,-eye.y) ;

	return vec4( mix( color*blend[1] , color_sun*blend[0] , d ) , 1.0 );
}


#shader "zone_sky_base"

#version 300 es
#version 330
#ifdef VERSION_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform mat4 inverse_modelview;
uniform mat4 inverse_projection;

uniform vec4 time;

#include "sky_function"


#ifdef VERTEX_SHADER

in vec3 a_vertex;

out vec4 v_pos;

void main()
{
	v_pos = vec4( a_vertex.xy , 1.0 , 1.0 );

	gl_Position = v_pos;
}

#endif
#ifdef FRAGMENT_SHADER

in vec4 v_pos;

out vec4 FragColor;

void main(void)
{
	vec3 v_eye = normalize( (inverse_modelview * inverse_projection * v_pos ).xyz ) ;
	
	FragColor=sky(v_eye,vec4(1.0));
}

#endif


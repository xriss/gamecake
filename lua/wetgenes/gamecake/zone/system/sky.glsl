
#header "sky_function"

uniform vec4 shadow_light; // normal of light, IE position of sun and power in W
uniform vec3 sun;
uniform vec3 moon;

#define PI 3.1415926538
#define PI2 (2.0*PI)
#define PIO2 (PI/2.0)


vec4 qset(float angle, vec3 axis) {
	return vec4( axis * sin( angle*0.5 ) , cos( angle*0.5 ) );
}
vec4 qmul(vec4 q1, vec4 q2) {
	return vec4( q2.xyz * q1.w + q1.xyz * q2.w + cross( q1.xyz , q2.xyz ) , q1.w * q2.w - dot( q1.xyz , q2.xyz ) );
}
vec3 qrot(vec3 v, vec4 q) {
	return qmul( q , qmul( vec4( v , 0 ) , q * vec4( -1, -1, -1, 1 ) ) ).xyz;
}




vec4 sky(vec3 eye, vec4 blend)
{
	float sunc=dot(eye,sun);
	float moonc=dot(eye,moon);
	
	float daynight=smoothstep( -0.1 , 0.1 , sun.y );

	vec4 color_sky=mix(     SRGB(vec4( 0.0 , 0.2 , 0.7 , 0.3 )) , SRGB(vec4( 0.0 , 0.0 , 0.1 , 0.3 )) , daynight );
	vec4 color_horizon=mix( SRGB(vec4( 0.2 , 0.2 , 0.5 , 0.3 )) , SRGB(vec4( 0.0 , 0.0 , 0.2 , 0.3 )) , daynight );
	vec4 color_floor=mix(   SRGB(vec4( 0.0 , 0.0 , 0.0 , 0.3 )) , SRGB(vec4( 0.0 , 0.0 , 0.0 , 0.3 )) , daynight );
	vec4 color_sun=SRGB(vec4( 1.0 , 0.7 , 0.3 , 0.8 ));
	vec4 color_moon=SRGB(vec4( 0.6 , 0.6 , 1.0 , 0.5 ));

	vec4 color=color_horizon;
	color=mix( color , color_sky   , pow( smoothstep(0.0,1.0, -eye.y) , 1.0 ) );
	color=mix( color , color_floor , pow( smoothstep(0.0,1.0,  eye.y) , 1.0 ) );
	color=color*blend[1];
	
	color=mix( color , color_sun  , smoothstep( 0.950 , 1.000 , sunc  )*blend[0] );
	color=mix( color , color_moon , smoothstep( 0.990 , 0.991 , moonc )*blend[0] );

	return vec4( color );
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

#include "gamecake_shader_funcs"

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
	
	FragColor=RGBS(sky(v_eye,vec4(1.0))*SRGB(color));
}

#endif


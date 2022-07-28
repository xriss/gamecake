
#shader "zone_floor_base"

#version 300 es
#version 330
#ifdef VERSION_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif
#if VERSION>120

#define IN in
#define OUT out

#else

#ifdef VERTEX_SHADER
#define IN attribute
#else
#define IN varying
#endif

#define OUT varying
#define texture texture2D
#define FragColor gl_FragColor

#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

//uniform mat4 inverse_modelview;
//uniform mat4 inverse_projection;

uniform mat4 camera;
uniform mat4 inverse_camera;

uniform vec4 time;

uniform vec4 offset; // center of grid



#ifdef VERTEX_SHADER

IN vec3 a_vertex;

OUT vec3 xy;
OUT vec3 eye;

OUT vec4 pos;

void main()
{
	vec4 v=vec4(a_vertex.xyz + offset.xyz, 1.0);
	xy=vec3(v.xz,length(a_vertex.xz));

	
	gl_Position = projection * modelview * v;

	pos=gl_Position;

	eye=normalize( ( modelview * camera * v ).xyz  - camera[3].xyz ) ;

}

#endif
#ifdef FRAGMENT_SHADER

IN vec4 pos;
IN vec3 eye;

IN vec3 xy;
OUT vec4 FragColor;

void main(void)
{
	float l=min( xy.z/256.0 , 1.0  );
	vec3 c1=mix( vec3( 0.5 , 0.5 , 0.5 ) , vec3( 0.5 , 0.5 , 0.5 ) , l );
	vec3 c2=mix( vec3( 0.6 , 0.6 , 0.6 ) , vec3( 0.5 , 0.5 , 0.5 ) , l );
	vec2 cheque=floor( (xy.xy-vec2(2.5))/5.0 );
	float a=mod(cheque.x+cheque.y,2.0);

	FragColor=vec4( mix(c1,c2,a) , 1.0 )*color;
}

#endif


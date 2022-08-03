
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

#shader "zone_floor_grass"

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

uniform mat4 camera;

uniform vec4 offset; // center of grid

uniform vec4  shadow_light;

uniform sampler2D tex;

uniform float idx;


#ifdef VERTEX_SHADER

IN vec4 a_normal;
IN vec3 a_vertex;
IN vec2 a_texcoord;

OUT vec2 v_texcoord;
OUT vec3 v_normal;
OUT vec4 v_position;
OUT vec4 v_relative;

void main()
{
	vec4 v=vec4( a_vertex.xyz , 1.0);

	gl_Position = projection * modelview * v;

	v_normal = normalize( mat3( camera * modelview ) * a_normal.xyz );
	v_texcoord=a_texcoord;
	v_position=camera * modelview * v;
	v_relative=modelview * v;
}

#endif
#ifdef FRAGMENT_SHADER

IN vec4 v_position;
IN vec4 v_relative;
IN vec3 v_normal;
IN vec4 v_color;
IN vec2  v_texcoord;

OUT vec4 FragColor;

// a noise that works on lower precision hardware (tested, but...)
float goldienoise(vec3 p)
{
	p+=step(0.0,p)*46.0-23.0; // move away from origin to reduce obvious patern
	float r=fract(tan(distance(p.xy*1.61803398874989484820459,p.xy)*p.z)*p.x);
    return r!=r ? 0.0 : r; // replace nan with 0
}

// with 2d blend
float goldienoise(in vec3 p,float s)
{
	vec3 fs=p/s; fs.z=floor(fs.z);
	vec3 ff=fract(fs);
    vec4 fc=vec4(floor(fs.xy),ceil(fs.xy))*s;
    return mix(
    mix( goldienoise( vec3( fc.xy , fs.z ) ),
         goldienoise( vec3( fc.xw , fs.z ) ),ff.y),
    mix( goldienoise( vec3( fc.zy , fs.z ) ),
         goldienoise( vec3( fc.zw , fs.z ) ),ff.y),ff.x);
}

void main(void)
{
	vec3 r=v_relative.xyz/v_relative.w;
	vec3 p=v_position.xyz/v_position.w;
	vec3 n=normalize(v_normal);
	vec3 s=shadow_light.xyz;
	vec2 uv=v_texcoord + vec2( dot(n,s)*0.5 ,0.0) ;
	
	float f=goldienoise(vec3(p.xz,64.0),1.0/16.0);
	uv.x+=(f-0.5)*smoothstep( -128.0, 0.0 , -length(r) )*0.75;

	FragColor = texture(tex, uv ).rgba * color;

}

#endif



#shader "zone_screen_bloom_draw"

#version 300 es
#version 330

#ifdef VERSION_ES
precision mediump float;
#endif


#shader "zone_screen_draw"

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif
  
uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

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
//	FragColor=vec4( texture(tex0, v_texcoord).rgb * texture(tex1, v_texcoord).rgb , 1.0 );
//	FragColor=vec4( texture(tex1, v_texcoord).rgb , 1.0 );
//	FragColor=vec4( texture(tex0, v_texcoord).rgb * (1.0-texture(tex1, v_texcoord).b) + texture(tex1, v_texcoord).g*0.25 , 1.0 );
//	FragColor=vec4( texture(tex0, v_texcoord).rgb * (1.0-texture(tex1, v_texcoord).b) + texture(tex1, v_texcoord).g*0.25 , 1.0 );

	vec3 m = texture(tex0, v_texcoord).rgb ;
	float s = texture(tex1, v_texcoord).r ;
	vec3 b = texture(tex2, v_texcoord).rgb ;

//	s = ( smoothstep( 0.0 , 0.5 , s )*2.0 + (1.0 - pow( (1.0-s) , 3.0 ) )*1.0 ) / 3.0;
//	s = smoothstep( 0.0 , 1.0 , s ) ;

	s = pow( s , 1.0/2.0 );

	vec3 c;
	
#if TWEAK==0

	c= m * s + b;

#elif TWEAK==1

	c= m ;

#elif TWEAK==2

	c=vec3( s,s,s );

#elif TWEAK==3

	c= m * s ;

#elif TWEAK==4

	c= b ;

#elif TWEAK==5

	c= m + b ;

#endif

	c=clamp(c,0.0,1.0);

#ifdef GAMMA

	FragColor=vec4( pow(c,vec3(1.0/float( GAMMA ))) , 1.0 );

#else

	FragColor=vec4( c , 1.0 );

#endif

}

#endif

#shader "zone_screen_build_occlusion"

#define PI 3.1415926538
#define PI2 (2.0*PI)

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

uniform sampler2D tex;

uniform mat4 inverse_projection;


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


in vec2 v_texcoord;

out vec4 FragColor;

// convert a uv into view space by sampling z buffer
vec3 depth_to_view(vec2 cc)
{
	vec4 p=inverse_projection * ( vec4( cc , texture(tex,cc).r , 1.0 )*2.0 - 1.0 );
	return p.xyz/p.w;
/*
	vec4 t=inverse_projection * ( vec4( cc ,  0.0  , 1.0 )*2.0 - 1.0 );
	t=t/t.w;

	vec3 n=t.xyz/t.z;

	float z=texture(tex,cc).r*2.0 - 1.0;

	float d =  - ( ( z*projection[3][3] - projection[3][2] ) / ( z*projection[2][3] - projection[2][2] ) );

	return n*d;
*/

}

// convert view space into depth space
vec3 view_to_depth(vec3 vv)
{
	vec4 p=projection * vec4( vv , 1.0);
	return (p.xyz/p.w)*0.5 + 0.5;
}

float random2d(vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float ambient_occlusion( vec2 vv )
{
#define AO_STEPS 3
#define AO_ANGLES 8
#define AO_SAMPLES AO_ANGLES*AO_STEPS
	float slen=0.2;
	
	vec2 texel_size = 1.0 / vec2( textureSize(tex,0) );

	vec3 p1=depth_to_view( vv );
	vec3 p2=view_to_depth( p1+vec3(slen,slen,0.0) );

	float dlen=length(p2.xy-vv.xy);

	float ac=0.0;
	for(int i=1;i<=AO_STEPS;i++)
	{
		float l=(float(i)-random2d(vv.yx)) /float(AO_STEPS);
		for(int ia=1;ia<=AO_ANGLES;ia++)
		{
			float r=( (float(ia)/float(AO_ANGLES)) + random2d(vv.xy) )*PI2;
			vec2 cc=vec2(sin(r),cos(r))*dlen*l;
			vec3 ss=depth_to_view(cc+vv);
			ac+=1.0-smoothstep( p1.z - slen , p1.z + slen , ss.z );
		}
	}
	return (ac/float(AO_SAMPLES));
}

uniform mat4 camera;
uniform mat4 shadow_mtx;
uniform sampler2D shadow_map;

float shadow_occlusion( vec2 vv )
{
	vec3 v=depth_to_view( vv );

	vec4 shadow_uv = shadow_mtx * camera * vec4(v,1.0) ;
	shadow_uv = (shadow_uv/shadow_uv.w) * 0.5 + 0.5;
	shadow_uv.w=0.0; // this should be normal into the light so we can adjust type of shadow
	
	const vec4 shadow=vec4(SHADOW);

	float shadow_value = 0.0; // max( 0.0 , shadow_uv.w );

	if( (shadow_uv.x > 0.0)  && (shadow_uv.x < 1.0) && (shadow_uv.y > 0.0) && (shadow_uv.y < 1.0) && (shadow_uv.z > 0.0) && (shadow_uv.z < 1.0) )
	{
		float shadow_tmp=0.0;
		float shadow_add=0.0;
		float shadow_min=1.0;
		vec2 shadow_texel_size = 1.0 / vec2( textureSize(shadow_map,0) );
		for(int x = -1; x <= 1; x++)
		{
			for(int y = -1; y <= 1; y++)
			{
				shadow_tmp = texture(shadow_map, shadow_uv.xy + vec2(float(x)-0.5+random2d(vv.xy),float(y)-0.5+random2d(vv.yx))*shadow_texel_size ).r ;
				shadow_add += shadow_tmp;
				shadow_min = min( shadow_min ,  shadow_tmp );
			}
		}
		shadow_value = max( shadow_value , smoothstep(	shadow[1] ,	shadow[2] ,
			shadow_uv.z - mix( shadow_min , shadow_add/9.0 , abs( shadow_uv.w ) ) ) );
	}
//	return	1.0-shadow_value;
	return ( (1.0-shadow_value)*shadow[0] + (1.0-shadow[0]) ) ;
}

void main(void)
{
	float s=shadow_occlusion(v_texcoord);
	float t=ambient_occlusion(v_texcoord);
	float d=pow( 1.0-smoothstep(0.0 , 0.5 , t ) , 1.0 );
	float l=pow( smoothstep(0.5 , 1.0 , t ) , 2.0 );
//	d=pow( smoothstep(0.0 , 1.0 , d ) , 1.0/1.0 )*0.4+0.6;
	FragColor=vec4( s*t, l , d , 1.0 );

}

#endif




#shader "zone_screen_build_bloom_pick"

#define PI 3.1415926538
#define PI2 (2.0*PI)

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif
  
uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex0;
//uniform sampler2D tex1;

uniform mat4 inverse_projection;


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
//	const vec3 one=vec3(1.0);
//	FragColor=vec4( one-pow( one-texture(tex,v_texcoord).rgb , vec3(1.0/4.0) ) , 1.0 );
	vec3 m = texture(tex0, v_texcoord).rgb ;
//	float s = texture(tex1, v_texcoord).r ;

//	s = ( smoothstep( 0.0 , 0.5 , s )*2.0 + (1.0 - pow( (1.0-s) , 3.0 ) )*1.0 ) / 3.0;

//	m = smoothstep( 0.5 , 1.0 , m );
//	float n=min(min(m.r,m.g),m.b);
//	m.rgb = vec3(n);

	FragColor=vec4( pow( m , vec3(4.0) ) , 1.0 );

}

#endif


#shader "zone_screen_build_blur"

#define PI 3.1415926538
#define PI2 (2.0*PI)

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif
  
uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex;

uniform mat4 inverse_projection;


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
	vec2 tc=v_texcoord;

	vec2 siz = 1.0 / vec2( textureSize(tex,0) );

#if BLUR_AXIS == 0
	siz.y=0.0;
#elif BLUR_AXIS == 1
	siz.x=0.0;
#endif

	vec4  c;

#if BLUR == -1

	c= min( min(
		texture(tex,tc             ) ,
		texture(tex,tc+siz.xy*  1.0) ) ,
		texture(tex,tc+siz.xy* -1.0) ) ;

#elif BLUR == 3

	c =texture(tex,tc).rgba*(1.0/3.0);
	c+=texture(tex,tc+siz.xy* 1.0).rgba*(1.0/3.0);
	c+=texture(tex,tc+siz.xy*-1.0).rgba*(1.0/3.0);

#elif BLUR == 5

	c =texture(tex,tc).rgba*(1.0/5.0);
	c+=texture(tex,tc+siz.xy* 1.0).rgba*(1.0/5.0);
	c+=texture(tex,tc+siz.xy*-1.0).rgba*(1.0/5.0);
	c+=texture(tex,tc+siz.xy* 2.0).rgba*(1.0/5.0);
	c+=texture(tex,tc+siz.xy*-2.0).rgba*(1.0/5.0);

#elif BLUR == 6

	c =texture(tex,tc).rgba*(2.0/6.0);
	c+=texture(tex,tc+siz.xy* 1.0).rgba*(1.0/6.0);
	c+=texture(tex,tc+siz.xy*-1.0).rgba*(1.0/6.0);
	c+=texture(tex,tc+siz.xy* 2.0).rgba*(1.0/6.0);
	c+=texture(tex,tc+siz.xy*-2.0).rgba*(1.0/6.0);

#elif BLUR == 22

	c =texture(tex,tc).rgba*(8.0/22.0);
	c+=texture(tex,tc+siz.xy* 1.0).rgba*(4.0/22.0);
	c+=texture(tex,tc+siz.xy*-1.0).rgba*(4.0/22.0);
	c+=texture(tex,tc+siz.xy* 2.0).rgba*(2.0/22.0);
	c+=texture(tex,tc+siz.xy*-2.0).rgba*(2.0/22.0);
	c+=texture(tex,tc+siz.xy* 3.0).rgba*(1.0/22.0);
	c+=texture(tex,tc+siz.xy*-3.0).rgba*(1.0/22.0);

#elif BLUR == 16

	c =texture(tex,tc).rgba*(2.0/16.0);
	c+=texture(tex,tc+siz.xy* 1.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy*-1.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy* 2.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy*-2.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy* 3.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy*-3.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy* 4.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy*-4.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy* 5.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy*-5.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy* 6.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy*-6.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy* 7.0).rgba*(1.0/16.0);
	c+=texture(tex,tc+siz.xy*-7.0).rgba*(1.0/16.0);

#elif BLUR_COUNT == 382

	c =texture(tex,tc).rgba*(128.0/382.0);
	c+=texture(tex,tc+siz.xy* 1.0).rgba*(64.0/382.0);
	c+=texture(tex,tc+siz.xy*-1.0).rgba*(64.0/382.0);
	c+=texture(tex,tc+siz.xy* 2.0).rgba*(32.0/382.0);
	c+=texture(tex,tc+siz.xy*-2.0).rgba*(32.0/382.0);
	c+=texture(tex,tc+siz.xy* 3.0).rgba*(16.0/382.0);
	c+=texture(tex,tc+siz.xy*-3.0).rgba*(16.0/382.0);
	c+=texture(tex,tc+siz.xy* 4.0).rgba*(8.0/382.0);
	c+=texture(tex,tc+siz.xy*-4.0).rgba*(8.0/382.0);
	c+=texture(tex,tc+siz.xy* 5.0).rgba*(4.0/382.0);
	c+=texture(tex,tc+siz.xy*-5.0).rgba*(4.0/382.0);
	c+=texture(tex,tc+siz.xy* 6.0).rgba*(2.0/382.0);
	c+=texture(tex,tc+siz.xy*-6.0).rgba*(2.0/382.0);
	c+=texture(tex,tc+siz.xy* 7.0).rgba*(1.0/382.0);
	c+=texture(tex,tc+siz.xy*-7.0).rgba*(1.0/382.0);

#endif

	FragColor=c.rgba;

}

#endif


#shader "zone_screen_build_dark"

#define PI 3.1415926538
#define PI2 (2.0*PI)

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif
  
uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform sampler2D tex;

uniform mat4 inverse_projection;


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
	vec2 tc=v_texcoord;

	vec3 siz = vec3( 1.0 / vec2( textureSize(tex,0) ) , 0.0 );

	vec4  c;

#if DARK == 1

	vec4 b=texture(tex,tc             );

	c= min( min( min( min(
		b ,
		texture(tex,tc+siz.xz*  1.0) ) ,
		texture(tex,tc+siz.xz* -1.0) ) ,
		texture(tex,tc+siz.zy*  1.0) ) ,
		texture(tex,tc+siz.zy* -1.0) ) ;

	c=(c+c+c+b)/4.0;

#elif DARK == 2

	c= 	texture(tex,tc             ) +
		texture(tex,tc+siz.xz*  1.0) +
		texture(tex,tc+siz.xz* -1.0) +
		texture(tex,tc+siz.zy*  1.0) +
		texture(tex,tc+siz.zy* -1.0) +
		texture(tex,tc+siz.xz*  2.0) +
		texture(tex,tc+siz.xz* -2.0) +
		texture(tex,tc+siz.zy*  2.0) +
		texture(tex,tc+siz.zy* -2.0) ;

	c=c/9.0;

#endif

	FragColor=c.rgba;

}

#endif
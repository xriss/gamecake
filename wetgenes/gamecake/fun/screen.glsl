
#shader "fun_screen_bloom_pick"
#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
    gl_Position = vec4(a_vertex, 1.0);
	v_texcoord=a_texcoord;
}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

uniform sampler2D tex;

uniform vec4  img_siz; /* 0,1 image size and 2,3 size of texture */
uniform vec4  img_off; /* texture offset (for sub layers) */

varying vec2  v_texcoord;

void main(void)
{
	vec4  c;
	c=texture2D(tex, v_texcoord).rgba;

// spread the bloom around rgb space a little
	c.r=0.25*(c.r*2.0+c.g    +c.b    );
	c.g=0.25*(c.r    +c.g*2.0+c.b    );
	c.b=0.25*(c.r    +c.g    +c.b*2.0);

	gl_FragColor=vec4( c.rgb*c.rgb , 1.0 );
}

#endif


#shader "fun_screen_bloom_blur"
#ifdef VERTEX_SHADER

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
    gl_Position = vec4(a_vertex, 1.0);
	v_texcoord=a_texcoord;
}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

uniform sampler2D tex;

varying vec2  v_texcoord;

uniform vec4  pix_siz; /* 0,1 pixel size */


void main(void)
{
	vec2 td;
	vec2  tc=v_texcoord;
	vec4  c;

	c =texture2D(tex,tc).rgba*(4.0/16.0);
	c+=texture2D(tex,tc+pix_siz.xy* 1.0).rgba*(3.0/16.0);
	c+=texture2D(tex,tc+pix_siz.xy*-1.0).rgba*(3.0/16.0);
	c+=texture2D(tex,tc+pix_siz.xy* 2.0).rgba*(2.0/16.0);
	c+=texture2D(tex,tc+pix_siz.xy*-2.0).rgba*(2.0/16.0);
	c+=texture2D(tex,tc+pix_siz.xy* 3.0).rgba*(1.0/16.0);
	c+=texture2D(tex,tc+pix_siz.xy*-3.0).rgba*(1.0/16.0);

	gl_FragColor=c.rgba;
}

#endif


#shader "fun_screen_bloom"
#ifdef VERTEX_SHADER

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

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

uniform sampler2D tex0;

varying vec2  v_texcoord;
varying vec4  v_color;

void main(void)
{
	gl_FragColor=vec4( texture2D(tex0, v_texcoord).rgb, 0.0 )*v_color;
}

#endif


#shader "fun_screen_scanline"
#ifdef VERTEX_SHADER

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

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

uniform sampler2D tex;

varying vec2  v_texcoord;
varying vec4  v_color;


const vec2 xo = vec2(1.0/256.0,0.0);
const vec2 ss = vec2(256.0,128.0);
const vec2 oo = vec2(1.0/256.0,1.0/128.0);

void main(void)
{
	vec2 tb;

	vec4  c,c2;
	
	float aa;

	tb=(floor(v_texcoord*ss)+vec2(0.5,0.5))*oo;

	c=texture2D(tex, tb).rgba;


	aa=2.0*(fract(v_texcoord.x*256.0)-0.5);
	if(aa<0.0)
	{
		c2=texture2D(tex, tb-xo ).rgba;
		aa=clamp(aa,-1.0,0.0);
		aa=aa*aa;
		c=mix(c,c2,aa*0.5);
	}
	else
	{
		c2=texture2D(tex, tb+xo).rgba;
		aa=clamp(aa,0.0,1.0);
		aa=aa*aa;
		c=mix(c,c2,aa*0.5);
	}


// scanline	
	aa=2.0*(fract(v_texcoord.y*128.0)-0.5);
	aa*=aa*aa*aa;
	c.rgb=c.rgb*(1.0-aa);
	
	gl_FragColor=vec4(c.rgb,1.0)*v_color;

}

#endif

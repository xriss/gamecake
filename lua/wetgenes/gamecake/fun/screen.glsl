
#shader "fun_screen_bloom_pick"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;
uniform sampler2D tex;
uniform vec4  img_siz; /* 0,1 image size and 2,3 size of texture */
uniform vec4  img_off; /* texture offset (for sub layers) */

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
    gl_Position = projection * vec4(a_vertex , 1.0);
	v_texcoord=a_texcoord;
}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

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

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;
uniform sampler2D tex;
uniform vec4  pix_siz; /* 0,1 pixel size */

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
    gl_Position = projection * vec4(a_vertex , 1.0);
	v_texcoord=a_texcoord;
}

#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

varying vec2  v_texcoord;


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


#shader "fun_screen_scanline"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;
uniform sampler2D tex;
uniform vec4 siz;
uniform vec4 vsiz; // output size in pixels x,y , rendersize in pixels z,w


#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * vec4(a_vertex, 1.0);
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

vec4 render(vec2 uv)
{
	vec2 xo = vec2(1.0/siz.x,0.0);
	
	vec2 tb;

	vec4  c,c2;
	
	float aa;

	tb=(floor(uv*siz.xy)+vec2(0.5,0.5))/siz.xy;

	c=texture2D(tex, tb).rgba;

	// reduce filter as output pixel size gets smaller
	vec2 fx=smoothstep( vec2(1.0,1.0) , vec2(5.0,5.0) , (vsiz.xy/vsiz.zw) );

	aa=2.0*(fract(uv.x*siz.x)-0.5);
	if(aa<0.0)
	{
		c2=texture2D(tex, tb-xo ).rgba;
		aa=clamp(aa,-1.0,0.0);
		aa=aa*aa;
		c=mix(c,c2,aa*0.5*fx.x);
	}
	else
	{
		c2=texture2D(tex, tb+xo).rgba;
		aa=clamp(aa,0.0,1.0);
		aa=aa*aa;
		c=mix(c,c2,aa*0.5*fx.x);
	}


// scanline	
	aa=2.0*(fract(uv.y*siz.y)-0.5);
	aa*=aa*aa*aa*fx.y;
	c.rgb=c.rgb*(1.0-aa);
	
	return vec4(c.rgb,1.0);

}

void main(void)
{
// do 4x sampling to try and smooth out any interferance, siz.zw is a display pixel in uv space
	gl_FragColor=	(
						render( v_texcoord               ) +
						render( v_texcoord+(siz.zw*0.25) ) +
						render( v_texcoord+(siz.zw*0.50) ) +
						render( v_texcoord+(siz.zw*0.75) ) 
					)*v_color*0.25;
}

#endif

#shader "fun_screen_dropshadow"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;
uniform sampler2D tex;
uniform vec4 siz;
uniform vec4  shadow_info; /* 0,1 tile size eg 8x8 and 2,3 the font texture size*/

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * vec4(a_vertex, 1.0);
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

vec4 render(vec2 uv)
{
	vec2 tb;

	vec4  c;

	tb=(floor(uv*siz.xy)+vec2(0.5,0.5))/siz.xy;

	c=texture2D(tex, tb).rgba;
	
	if	(	(c.a<texture2D(tex, tb+(vec2(-shadow_info[0], shadow_info[0])/siz.xy)).a)	)
	{ 
		c=(c*shadow_info[1]);
		c.a=c.a+(1.0-shadow_info[1]);
	}
	else
	if	(	(c.a<texture2D(tex, tb+(vec2(-shadow_info[2], shadow_info[2])/siz.xy)).a)	)
	{
		c=(c*shadow_info[3]);
		c.a=c.a+(1.0-shadow_info[3]);
	}
	
	return c;
}

void main(void)
{
	gl_FragColor=	render(v_texcoord)*v_color;
}

#endif

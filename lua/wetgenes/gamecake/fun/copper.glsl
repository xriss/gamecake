

#shader "fun_copper_back_y5"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;
// 5 colors to blend vertically
uniform	vec4 cy0;
uniform	vec4 cy1;
uniform	vec4 cy2;
uniform	vec4 cy3;
uniform	vec4 cy4;
uniform	vec4 sizpos;

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

void main(void)
{
	vec2 uv=(v_texcoord+sizpos.wz)/sizpos.xy;
	vec4 c;
	
	if(uv.y<0.00)
	{
		c=cy0;
	}
	else
	if(uv.y<0.25)
	{
		c=mix(cy0,cy1,uv.y*4.0);
	}
	else
	if(uv.y<0.50)
	{
		c=mix(cy1,cy2,(uv.y-0.25)*4.0);
	}
	else
	if(uv.y<0.75)
	{
		c=mix(cy2,cy3,(uv.y-0.50)*4.0);
	}
	else
	if(uv.y<1.00)
	{
		c=mix(cy3,cy4,(uv.y-0.75)*4.0);
	}
	else
	{
		c=cy4;
	}
	
	
	gl_FragColor=c*v_color;

}

#endif



#shader "fun_copper_back_y3"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;
// 3 colors to blend vertically
uniform	vec4 cy0;
uniform	vec4 cy1;
uniform	vec4 cy2;
uniform	vec4 sizpos;

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

void main(void)
{
	vec2 uv=(v_texcoord+sizpos.wz)/sizpos.xy;
	vec4 c;
	
	if(uv.y<0.00)
	{
		c=cy0;
	}
	else
	if(uv.y<0.5)
	{
		c=mix(cy0,cy1,uv.y*2.0);
	}
	else
	if(uv.y<1.00)
	{
		c=mix(cy1,cy2,(uv.y-0.5)*2.0);
	}
	else
	{
		c=cy2;
	}
	
	
	gl_FragColor=c*v_color;

}

#endif

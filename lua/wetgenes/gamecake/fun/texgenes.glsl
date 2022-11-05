

#header "texgenes_shadertoy_head"

#version 300 es
#version 330
#ifdef VERSION_ES
precision highp float;
precision highp int;
#endif

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click

// a define to test for and a new camera input to use in raymarching
//#define ICAMERA 1
//uniform mat4 iCamera; // Camera transform

// a define to test for and a new camera input to use in 2d views
//#define ICAMERA2D 1
//uniform mat4 iCamera2D; // 2D Camera transform

#define HW_PERFORMANCE 0

#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2  v_texcoord;
//out vec4  gl_Position;

void main()
{
	gl_Position = vec4(a_vertex.xyz, 1.0) ;
	v_texcoord = a_texcoord;
}

#endif


#ifdef FRAGMENT_SHADER

in vec2  v_texcoord;
//in vec4  gl_Position;

out vec4 FragColor;

#endif



#header "texgenes_shadertoy_foot"

#ifdef FRAGMENT_SHADER

void main()
{
	mainImage( FragColor , v_texcoord );
}

#endif



#shader "texgenes_test"

#include "texgenes_shadertoy_head"

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	fragColor=vec4(1.0,1.0,1.0,1.0);
}

#include "texgenes_shadertoy_foot"


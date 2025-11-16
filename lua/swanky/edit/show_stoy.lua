--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")
local function ls(...) print(wstr.dump({...})) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- keep track of all the open documents

M.bake=function(oven,show_stoy)
	local show_stoy=show_stoy or {}
	show_stoy.oven=oven
	
	show_stoy.modname=M.modname

	local gl=oven.gl

	local show=oven.rebake(oven.modname..".show")
	local gui=oven.rebake(oven.modname..".gui")


	local shadertoy_head=[[

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
#define ICAMERA 1
uniform mat4 iCamera; // Camera transform

// a define to test for and a new camera input to use in 2d views
#define ICAMERA2D 1
uniform mat4 iCamera2D; // 2D Camera transform

#define HW_PERFORMANCE 0

#ifdef VERTEX_SHADER

in vec3 a_vertex;
in vec2 a_texcoord;

out vec2  v_texcoord;
//out vec4  gl_Position;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

void main()
{
	gl_Position = projection * modelview * vec4(a_vertex.xyz, 1.0) ;
	v_texcoord = a_texcoord;
}

#endif


#ifdef FRAGMENT_SHADER

in vec2  v_texcoord;
//in vec4  gl_Position;

out vec4 FragColor;


#line 1
]]

	local shadertoy_tail=[[
void main()
{
	mainImage( FragColor , v_texcoord );
}
#endif
]]

--[[

uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform sampler2d iChannel0;             // input channel. XX = 2D/Cube
uniform sampler2d iChannel1;             // input channel. XX = 2D/Cube
uniform sampler2d iChannel2;             // input channel. XX = 2D/Cube
uniform sampler2d iChannel3;             // input channel. XX = 2D/Cube
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)

]]


	show_stoy.start=function(str)

		local pname="swanky_edit_show_stoy"

		show_stoy.itime=0

		gl.headers[pname]=shadertoy_head..str..shadertoy_tail
		gl.program_source(pname,{source=gl.headers[pname]})
		local suc,err = pcall( function() gl.program(pname) end )

		if not suc then
			show.set_error(err)
		end

	end
	show_stoy.itime=0
	show_stoy.widget_draw=function(px,py,hx,hy) -- draw a widget of this size using opengl

--		show.fbo:resize(hx,hy,0)

		local pname="swanky_edit_show_stoy"

		local state=gui.datas.get_string("run_state")
		if state~="play" and state~="pause" then return end -- nothing to draw
		if state=="play" then -- update and draw
			show_stoy.itime=show_stoy.itime+1
		end

--		gui.master.ids.runtext.hidden=true

--		if i%60 == 0 then print(i/60) end
--		print(hx,hy)
		
		gl.Color(1,1,1,1)

		gl.state.push(gl.state_defaults)
		gl.state.set({
			[gl.DEPTH_WRITEMASK]			=	gl.FALSE,
			[gl.DEPTH_TEST]					=	gl.FALSE,
			[gl.CULL_FACE]					=	gl.FALSE,
		})


pcall(function()
		local t={
			px,		py,		0,		0,	hy,
			px+hx,	py,		0,		hx,	hy,
			px,		py+hy,	0,		0,	0,
			px+hx,	py+hy,	0,		hx,	0,
		}
		oven.cake.canvas.flat.tristrip("xyzuv",t,pname,function(p)

--[[
				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				show.fbo:bind_texture()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
]]

-- shadertoy compatability
			gl.Uniform3f( p:uniform("iResolution"), hx,hy,0 )
			gl.Uniform1f( p:uniform("iTime"), show_stoy.itime/60 )

			gl.UniformMatrix4f( p:uniform("iCamera"), show.cam )
			gl.UniformMatrix4f( p:uniform("iCamera2D"), show.cam2d )

--			if show.mouse then
				gl.Uniform4f( p:uniform("iMouse"), show.mouse[1] , show.mouse[2] , show.mouse[3] , show.mouse[4] )
--			else
--				gl.Uniform4f( p:uniform("iMouse"), hx/2 , hy/2 , -hx/2 , -hy/2 )
--			end

		end)
end)
		
		gl.state.pop()

	end
	
	return show_stoy
end

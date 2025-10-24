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

-- create a u16 string of all 16bit numbers in sequence (128k) for vertex buffer
local vgen64=function()
	local tab={}
	for b=0,255 do
		local s=""
		for a=0,240,16 do
			s=s..string.char(
				a+0,b,a+1,b,a+2,b,a+3,b,
				a+4,b,a+5,b,a+6,b,a+7,b,
				a+8,b,a+9,b,a+10,b,a+11,b,
				a+12,b,a+13,b,a+14,b,a+15,b)
		end
		tab[#tab+1]=s
	end
	return table.concat(tab)
end

-- keep track of all the open documents

M.bake=function(oven,show_vtoy)
	local show_vtoy=show_vtoy or {}
	show_vtoy.oven=oven
	
	show_vtoy.modname=M.modname

	local gl=oven.gl

	local show=oven.rebake(oven.modname..".show")
	local gui=oven.rebake(oven.modname..".gui")
	
	local fats=require("wetgenes.fats")

	local vapd=oven.cake.canvas.flat.array_predraw(
		{
			dataraw=vgen64(),
			datasize=0x20000-2, -- 21845 triangles ( 6 bytes per tri )
			pstride=2,
			array=gl.TRIANGLES,
			vb=-1,
		}
	)

	local vtoy_head=[[

#version 330
#version 300 es
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

// a define to test for and a new camera input to use in 3d views
#define ICAMERA3D 1
uniform mat4 iCamera3D; // 3D Camera transform

#define HW_PERFORMANCE 0

uniform vec4 color;
uniform mat4 modelview;
uniform mat4 projection;

#ifdef VERTEX_SHADER

// allow discard in a vertex shader as a return rather than an error
#define discard return

#endif

#line 1
]]

	local vtoy_tail=[[

#ifdef VERTEX_SHADER

in float a_idx;

out vec3  v_xyz;
out vec3  v_nrm;
out vec4  v_uvst;

void main()
{
	
	mainVertex( v_xyz , v_nrm , v_uvst , a_idx );

	v_nrm = mat3(modelview) * v_nrm ; // only rotate normal

	gl_Position = projection * modelview * vec4( v_xyz , 1.0 );
}

#endif


#ifdef FRAGMENT_SHADER

in vec3  v_xyz;
in vec3  v_nrm;
in vec4  v_uvst;

out vec4 FragColor;	

void main()
{
	mainFragment( FragColor , v_xyz , v_nrm , v_uvst );
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

local i=0
	show_vtoy.widget_draw=function(px,py,hx,hy) -- draw a widget of this size using opengl

--print(px,py,hx,hy)

--		show.fbo:resize(hx,hy,0)

		local pname="swanky_edit_show_vtoy"

		gl.headers[pname]=vtoy_head..gui.master.ids.texteditor.txt.get_text()..vtoy_tail
		gl.program_source(pname,{source=gl.headers[pname]})
		local suc,err = pcall( function() gl.program(pname) end )

		if not suc then

			gui.master.ids.runtext.txt.set_text(err,"error.txt")
			gui.master.ids.runtext.txt.set_lexer()

			gui.master.ids.runtext.hidden=false

--			print(err)
			return

		end

		gui.master.ids.runtext.hidden=true

		i=i+1
--		if i%60 == 0 then print(i/60) end
--		print(hx,hy)
		
		gl.Color(1,1,1,1)

		gl.state.push(gl.state_defaults)

		gl.state.set({
			[gl.DEPTH_WRITEMASK]			=	gl.TRUE,
			[gl.DEPTH_TEST]					=	gl.TRUE,
			[gl.CULL_FACE]					=	gl.FALSE,
		})

		local fbo=gui.master.ids.runfbo.fbo
		if fbo.w~=hx or fbo.h~=hy then -- resize so we need a new fbo
			fbo:resize(hx,hy,4096)
		end

		local view=oven.cake.views.create({
			mode="fbo",
			fbo=fbo,
			vx=fbo.w,
			vy=fbo.h,
			vz=fbo.h*2,
			fov=1,
			cx=0.5,
			cy=0.5,
		})
		oven.cake.views.push_and_apply(view)

	gl.ClearColor(0,0,0,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
		
--		gl.Translate(show.pos)
--		gl.Rotate( show.rot[1] , {0,1,0} )
--		gl.Rotate( show.rot[2] , {1,0,0} )


		gl.LoadMatrix(show.cam3d)
--		gl.LoadMatrix(show.cam)


local suc,err=pcall(function()

		vapd.draw(function(p)

			do
				local a=p:attrib("a_idx")
				if a>=0 then
					gl.VertexAttribPointer(a,1,gl.UNSIGNED_SHORT,gl.FALSE,2,0)
--					gl.VertexAttribIPointer(a,1,gl.UNSIGNED_SHORT,2,0)
					gl.EnableVertexAttribArray(a)
				end
			end

-- shadertoy compatability
			gl.Uniform3f( p:uniform("iResolution"), hx,hy,0 )
			gl.Uniform1f( p:uniform("iTime"), i/60 )

			gl.UniformMatrix4f( p:uniform("iCamera"), show.cam )
			gl.UniformMatrix4f( p:uniform("iCamera2D"), show.cam2d )
			gl.UniformMatrix4f( p:uniform("iCamera3D"), show.cam3d )

			gl.Uniform4f( p:uniform("iMouse"), show.mouse[1] , show.mouse[2] , show.mouse[3] , show.mouse[4] )
		end,pname)
		gl.CheckError()

end)
		if not suc then

			gui.master.ids.runtext.txt.set_text(err,"error.txt")
			gui.master.ids.runtext.txt.set_lexer()

			gui.master.ids.runtext.hidden=false

		end


		oven.cake.views.pop_and_apply()
		gl.state.pop()

	end
	
	return show_vtoy
end

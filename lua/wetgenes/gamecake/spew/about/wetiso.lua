--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


-- This curve used here is the curvy part of splines.
-- input 0 to 1 and output 0 to 1 smoothing into and from the 0 and 1  (like a sinwave going from -1 to +1)
local function spine(a)
	local aa=a*a
	return ((aa+(aa*2))-((aa*a)*2))
end
-- input 0 to 1 and output 0 to 1 to 0 so it loops smoothly
local function spine2(n)
	local n,a=math.modf(n) -- just want the fractional part
	a=a*2
	if a>1 then a=2-a end
	local aa=a*a
	return ((aa+(aa*2))-((aa*a)*2))
end



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,about)
	about=about or {}

	local gl=oven.gl
	local cake=oven.cake
	
	local sounds=cake.sounds
	local canvas=cake.canvas
	local fbs=cake.framebuffers
	
	local flat=canvas.flat
	local font=canvas.font
	
	local wetiso=oven.rebake("wetgenes.gamecake.spew.geom_wetiso")
	local geom=oven.rebake("wetgenes.gamecake.spew.geom")


	local opts={
		width=640,
		height=480,
	}
	
	about.layout=cake.layouts.create{}
	
	about.setup=function()

		if oven.last and oven.last~=about then about.exit=oven.last end -- remeber where we came from

		wetiso.setup()
		
		if about.playtune then
			about.playtune()
		end

		about.t=0
		
	end
	
	about.clean=function()
	end
	
	about.update=function()
		about.t=about.t+1
	end
	
	about.draw=function()

		about.layout.apply(opts.width,opts.height,1/4,opts.width*4,"clip")

		canvas.gl_default() -- reset gl state

		gl.ClearColor(gl.C4(0x0000))
		gl.Clear(gl.COLOR_BUFFER_BIT)--+gl.DEPTH_BUFFER_BIT)

		gl.PushMatrix()

			gl.Translate(320,240,0)
			gl.Scale(320,320,320)
			gl.Rotate(about.t,0,-1,0)
			gl.Rotate(about.t/8,1,0,0)
			gl.Enable(gl.CULL_FACE)
			gl.Color(0,0.25,0.75,1)
			wetiso.draw()
			gl.Disable(gl.CULL_FACE)
		
		gl.PopMatrix()

	end

	function about.msg(m)
	
		if m.class=="key" or m.class=="mouse" or m.class=="joykey" then
			if m.action==-1 then
				if about.exitname then
					oven.next=oven.rebake(about.exitname)
				else
					if about.exit then oven.next=about.exit end
				end
			end
		end

	end

	return about
end

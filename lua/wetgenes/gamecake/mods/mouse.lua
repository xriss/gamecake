--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local buffedit=require("wetgenes.gamecake.mods.console.buffedit")


function M.bake(oven,mouse)

	mouse=mouse or {}
	mouse.modname=M.modname
	
	mouse.active=false

	local win=oven.win
	local cake=oven.cake
	local gl=oven.gl

	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local layout=cake.layouts.create{} -- fullscreen

	function mouse.setup()
	
		mouse.input={
			x=0,
			y=0,
		} -- current state of the raw posix mouse inputs

		mouse.x=0
		mouse.y=0
		mouse.xraw=0
		mouse.yraw=0
		
		if win.flavour=="raspi" then -- we need to manage a mouse curser and input
			mouse.active=true
		end
		
	end

	function mouse.clean()
	
	end
	
	function mouse.update()
	
		if not mouse.active then return end
		
	end
	
	function mouse.draw()
	
		if not mouse.active then return end
		
		layout.apply()

--		layout.viewport() -- did our window change?
--		layout.project23d(layout.w,layout.h,1/4,layout.h*4)
		
		canvas.gl_default() -- reset gl state

--		gl.MatrixMode(gl.PROJECTION)
--		gl.LoadMatrix( layout.pmtx )

--		gl.MatrixMode(gl.MODELVIEW)
--		gl.LoadIdentity()
--		gl.Translate(-w/2,-h/2,-h) -- top/left 1unit==1pixel
		gl.PushMatrix()

		local wh=layout.w*layout.h
		local ss=math.ceil(math.sqrt(wh)/1000)
		

--		flat.quad(mouse.xraw,mouse.yraw,mouse.xraw+16*ss,mouse.yraw+16*ss)
		
		local x,y=mouse.xraw,mouse.yraw

		local dat1={
			x+12*ss,	y+12*ss,	0,		0.75,0.75,0.75,0.75,
			x+5*ss,		y+12*ss,	0,		0.75,0.75,0.75,0.75,
			x,			y,			0,		1,1,1,1,
			x,			y+17*ss,	0,		0.75,0.75,0.75,0.75,
			
			}
			
		local dat2={
			x+14*ss,		y+13*ss,	0,
			x+5*ss,			y+13*ss,	0,
			x-1*ss,			y-2*ss,		0,
			x-1*ss,			y+19*ss,	0,
			
			}
		
		gl.Color(pack.argb4_pmf4(0xf000))
		flat.tristrip("xyz",dat2)

		gl.Color(pack.argb4_pmf4(0xffff))			
		flat.tristrip("xyzrgba",dat1)

		gl.PopMatrix()


	end

	
	function mouse.msg(m)

--print(m.class)

		if m.class=="key" then
		
			
		elseif m.class=="mouse" then
		
			mouse.xraw=m.xraw or m.x
			mouse.yraw=m.yraw or m.y
			mouse.x=m.x
			mouse.y=m.y
			
			
		end
		
		return m
	end

	return mouse
end

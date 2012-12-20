-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local buffedit=require("wetgenes.gamecake.mods.console.buffedit")


function M.bake(state,mouse)

	mouse=mouse or {}
	mouse.modname=M.modname

	local win=state.win
	local cake=state.cake
	local gl=cake.gl

	local canvas=state.canvas.child()
	canvas.layout=false
	local font=canvas.font
	local flat=canvas.flat

	function mouse.setup()

		mouse.x=0
		mouse.y=0
		mouse.xraw=0
		mouse.yraw=0
		
		mouse.show=false

	end

	function mouse.clean()
	
	end
	
	function mouse.update()
	
		if mouse.show then
		end
		
	end
	
	function mouse.draw()
	

		local w,h=state.win.width,state.win.height
		gl.Viewport(0,0,w,h)
		canvas.gl_default() -- reset gl state

		gl.MatrixMode(gl.PROJECTION)
		gl.LoadMatrix( tardis.m4_project23d(w,h,w,h,0.5,h*2) )

		gl.MatrixMode(gl.MODELVIEW)
		gl.LoadIdentity()
		gl.Translate(-w/2,-h/2,-h) -- top/left 1unit==1pixel
		gl.PushMatrix()

		local wh=w*h
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
	
		if m.class=="key" then
		
--			console.keypress(m.ascii,m.keyname,m.action)
			
		elseif m.class=="mouse" then
		
			mouse.xraw=m.xraw or m.x
			mouse.yraw=m.yraw or m.y
			mouse.x=m.x
			mouse.y=m.y
			
--			console.keypress(m.action,m.x,m.y,m.keycode)
			
		end
	end

	return mouse
end

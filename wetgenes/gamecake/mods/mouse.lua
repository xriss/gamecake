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


function M.bake(oven,mouse)

	mouse=mouse or {}
	mouse.modname=M.modname
	
	mouse.active=false

	local win=oven.win
	local cake=oven.cake
	local gl=cake.gl

	local canvas=cake.canvas.child()
	canvas.layout=false
	local font=canvas.font
	local flat=canvas.flat

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

		local w,h=oven.win.width,oven.win.height
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
	
		if not mouse.active then return m end
	
if m.class=="posix_mouse" then

	local function adjust(dx,dy)

		local inp=mouse.input
		
		inp.x=inp.x+dx
		inp.y=inp.y+dy
		
		if inp.x<0   then inp.x=0    end
		if inp.y<0   then inp.y=0    end
		if inp.x>=win.width  then inp.x=win.width-1 end
		if inp.y>=win.height then inp.y=win.height-1 end
		
		win:push_msg{time=os.time(),class="mouse",x=inp.x,y=inp.y,action=0,keycode=0}
		
	end
	local function click(code,act)

		local inp=mouse.input

		win:push_msg{time=os.time(),class="mouse",x=inp.x,y=inp.y,action=act,keycode=code}

	end

--	print(m.class,m.type,m.code,m.value)

	if m.type==2 then -- movement
	
		local v=m.value
		if v >= 0x80000000 then v=v-0x100000000 end

		if m.code==0 then -- x
			adjust(v,0)
		elseif m.code==1 then -- y
			adjust(0,v)			
--		elseif m.code==8 then -- mouse wheel
--			print("z",v)
		end
		
	elseif m.type==1 then -- buttons
	
		local k=1+m.code-272 -- 1 is the left mouse button
		
		if m.value==0 then --up
			click(k,-1)
		elseif m.value==1 then --down
			click(k, 1)		
		end
	
	end
	
	return nil -- we ate it
end

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

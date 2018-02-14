--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- do a reverse enum lookup, get a name from a value
local lookup=function(tab,num)
	for n,v in pairs(tab) do
	if v==num then
		return n end
	end
end

local sdl={}

sdl.pads_map={}
sdl.pads={}

-- build a map of pads idx to player idx
sdl.pads_fix=function()
	local idx=0
	for i=1,#sdl.pads do
		local v=sdl.pads[i]
		if type(v)~="string" then
			idx=idx+1
			sdl.pads_map[i]=idx
		else
			sdl.pads_map[i]=nil
		end
	end
end

local SDL=require("SDL")

local wstr=require("wetgenes.string")
local dprint=function(a) print(wstr.dump(a)) end

sdl.print=print

sdl.video_init_done=false
sdl.video_init=function()
	if not sdl.video_init_done then
		sdl.video_init_done=true
		SDL.init{ SDL.flags.Video, SDL.flags.Joystick, SDL.flags.GameController, SDL.flags.Events, }
--		SDL.init{ SDL.flags.Everything, }
--		SDL.videoInit()
	end
end

sdl.screen=function()
--	print("SDL screen")
	sdl.video_init()
	local b=SDL.getDisplayBounds(0)
	
-- PI SDL may return nil at this point?
-- in which case just lie and return 640x480
	if not b then return 640,480 end

	return b.w,b.h
end


sdl.create=function(t)
--	print("SDL create")
	sdl.video_init()

	if not t.console then -- try and free the windows console unless asked not to
		
		pcall( function() -- we dont care if this fails, just means we are not running on windows
		
			local ffi=require("ffi")
			ffi.cdef(" void FreeConsole(void); ")
			ffi.C.FreeConsole()

		end)
		
	end

--[[
	SDL.glSetAttribute(SDL.glAttr.RedSize, 1);
	SDL.glSetAttribute(SDL.glAttr.GreenSize, 1);
	SDL.glSetAttribute(SDL.glAttr.BlueSize, 1);
	SDL.glSetAttribute(SDL.glAttr.DepthSize, 1);
	SDL.glSetAttribute(SDL.glAttr.DoubleBuffer, 1);
]]

	local it={}
	sdl.it=it
	
	local flags={ SDL.window.Resizable , SDL.window.OpenGL }
--[[
	if     view=="full" then	 flags={SDL.window.Desktop,SDL.window.OpenGL}
	elseif view=="max"  then	 flags={SDL.window.Maximized,SDL.window.OpenGL}
	end
]]
	
	it.win= assert(SDL.createWindow {
		title   = t.title,
		width   = t.width,
		height  = t.height,
		flags   = flags,--{ SDL.window.Resizable , SDL.window.OpenGL },
		x       = t.x,
		y       = t.y,
	})
	
	return it
end

sdl.destroy=function(it)
--	print("SDL destroy")
	if it then
		SDL.destroyWindow(it.win)
	elseif sdl.it then
		SDL.destroyWindow(sdl.it.win)
		sdl.it=nil
	end
	return nil
end

sdl.info=function(it,w)
--	print("SDL info")
	it=it or sdl.it
	w=w or {}
	w.width,w.height=it.win:getSize()
end

sdl.resize=function(it,w,h)
	it=it or sdl.it
	if it and it.win then it.win:setSize(w,h) end
end

sdl.show=function(it,view)
--	print("SDL show")
	it=it or sdl.it
	it.win:show()
	
	if     view=="full" then	 it.win:setFullscreen(SDL.window.Desktop)
	elseif view=="max"  then	 it.win:setFullscreen(SDL.window.Maximized)
	else						 it.win:setFullscreen(0)
	end
	
--	sdl.mousexy_init()
	
	return nil
end

sdl.context=function(it)
--	print("SDL context")
	it=it or sdl.it
	it.ctx=assert(SDL.glCreateContext(it.win))
	SDL.glMakeCurrent(it.win, it.ctx)
	SDL.glSetSwapInterval(1) -- enable vsync by default
	return it.ctx
end

sdl.swap=function(it)
--	print("SDL swap")
	it=it or sdl.it
	SDL.glSwapWindow(it.win)
end
	

sdl.sleep=function(ms)
--	print("SDL sleep")
	SDL.delay(ms)
	return nil
end

sdl.time=function()
--	print("SDL time")
	return SDL.getTicks()/1000
end

sdl.queue={}
sdl.msg=function()
--	print("SDL msg")
	sdl.msg_fetch()
	if sdl.queue[1] then
		return table.remove(sdl.queue,1)
	end
end

sdl.peek=function()
--	print("SDL peek")
	sdl.msg_fetch()
	return sdl.queue[1]
end

sdl.wait=function()
	print("SDL wait")
	return nil
end

sdl.mousexy={0,0}

-- add current mouse position to the queue
-- this does not help, we still get 0,0 at the start until the mouse moves...
--[[
sdl.mousexy_init=function()
	local b,x,y=SDL.getMouseState()
	local t={}
	t.time=sdl.time()
	t.class="mouse"
	t.action=0
	t.x=x
	t.y=y
	sdl.queue[#sdl.queue+1]=t

	sdl.mousexy[1]=t.x
	sdl.mousexy[2]=t.y
	
print("MOUSE",t.x,t.y)

end
]]

sdl.msg_fetch=function()

	for e in SDL.pollEvent() do

		if     	(e.type == SDL.event.KeyDown) or 
				(e.type == SDL.event.KeyUp) then

			local t={}
			t.time=sdl.time()
			t.class="key"
			t.action=(e.type==SDL.event.KeyUp) and -1 or ( e["repeat"] and 0 or 1 )
			t.keyname=SDL.getKeyName(e.keysym.sym)
			
--			dprint(t)

			sdl.queue[#sdl.queue+1]=t
					
		elseif     (e.type == SDL.event.TextInput) then

			local t={}
			t.time=sdl.time()
			t.class="text"
			t.text=e.text

			sdl.queue[#sdl.queue+1]=t

		elseif	(e.type == SDL.event.MouseWheel)  then
		
--			dprint(e)
		
			local t={}
			t.time=sdl.time()
			t.class="mouse"
			t.action=0
			t.x=sdl.mousexy[1]
			t.y=sdl.mousexy[2]
			if 		e.y>0 then	t.keycode=4 t.action=-1
			elseif 	e.y<0 then	t.keycode=5 t.action=-1 end
			
--			dprint(t)

			sdl.queue[#sdl.queue+1]=t

		elseif	(e.type == SDL.event.MouseButtonDown) or 
				(e.type == SDL.event.MouseButtonUp) or 
				(e.type == SDL.event.MouseMotion) then

			local t={}
			t.time=sdl.time()
			t.class="mouse"
			t.action=(e.type==SDL.event.MouseButtonUp) and -1 or ( (e.type==SDL.event.MouseButtonDown) and 1 or 0 )
			t.x=e.x
			t.y=e.y
			t.keycode=e.button
			
--			dprint(t)

			sdl.queue[#sdl.queue+1]=t

			sdl.mousexy[1]=e.x
			sdl.mousexy[2]=e.y
	
		
		elseif	(e.type == SDL.event.FingerDown) or 
				(e.type == SDL.event.FingerUp) or 
				(e.type == SDL.event.FingerMotion) then

			local t={}
			t.time=sdl.time()
			t.class="touch"
			t.action=(e.type==SDL.event.FingerUp) and -1 or ( (e.type==SDL.event.FingerDown) and 1 or 0 )
			t.x=e.x
			t.y=e.y
			t.id=e.fingerId
			t.pressure=e.pressure
			
--			dprint(t)

			sdl.queue[#sdl.queue+1]=t

--			sdl.mousexy[1]=e.x
--			sdl.mousexy[2]=e.y
	
		
		elseif	(e.type == SDL.event.WindowEvent) then -- ignore

		elseif	(e.type == SDL.event.Quit) then -- window close button, or alt f4

			local t={}
			t.time=sdl.time()
			t.class="close"

			sdl.queue[#sdl.queue+1]=t

		elseif	(e.type == SDL.event.JoyDeviceAdded) then --ignore
		elseif	(e.type == SDL.event.ControllerDeviceAdded) then
		
			sdl.pads[#sdl.pads+1]=SDL.gameControllerOpen(e.which)
			sdl.pads_fix()

--print("SDL","Open",e.which)

		elseif	(e.type == SDL.event.JoyDeviceRemoved) then --ignore
		elseif	(e.type == SDL.event.ControllerDeviceRemoved) then

			sdl.pads[e.which+1]="CLOSED"
			sdl.pads_fix()
			
--print("SDL","Close",e.which)

		elseif	(e.type == SDL.event.JoyAxisMotion) then --ignore
		elseif	(e.type == SDL.event.ControllerAxisMotion) then

			local dev=sdl.pads[e.which+1]
			local id=sdl.pads_map[e.which+1]

--print("SDL","AXIS",e.which,id,lookup(SDL.controllerAxis,e.axis),e.value)

			local t={}
			t.time=sdl.time()
			t.class="padaxis"
			t.value=e.value
			t.code=e.axis
			t.name=lookup(SDL.controllerAxis,e.axis)
			t.id=id

			sdl.queue[#sdl.queue+1]=t
			
		elseif	(e.type == SDL.event.JoyButtonDown)        or (e.type == SDL.event.JoyButtonUp)        then --ignore
		elseif	(e.type == SDL.event.ControllerButtonDown) or (e.type == SDL.event.ControllerButtonUp) then

			local dev=sdl.pads[e.which+1]
			local id=sdl.pads_map[e.which+1]

--print("SDL","KEY",e.which,id,lookup(SDL.controllerButton,e.button),e.state)

			local t={}
			t.time=sdl.time()
			t.class="padkey"
			t.value=e.state and 1 or -1
			t.code=e.button
			t.name=lookup(SDL.controllerButton,e.button)
			t.id=id

			sdl.queue[#sdl.queue+1]=t

		else

-- handle JoyHatMotion ?


			local s=lookup(SDL.event,e.type)
--			print(s.." : "..e.type)
--			dprint(e)

		end
	end

end


sdl.warp_mouse=function(it,x,y)
	it=it or sdl.it
	it.win:warpMouse(x,y)
	return true
end

--
-- kinda hacky, but turn a grd into a surface via a bitmap structure...
--
local _grd_to_surface=function(g)

	local wgrd=require("wetgenes.grd")
	local pack=require("wetgenes.pack")

	if type(g)=="string" then -- assume swanky32 bitdown format and convert to grd

		g=require("wetgenes.gamecake.fun.bitdown").pix_grd(g) -- convert to grd

	end


	assert(g:convert(wgrd.U8_RGBA)) -- make sure it is this format
	assert(g:flipy()) -- needs to be the other way up
	
	local argb=g:pixels(0,0,g.width,g.height)
	for i=0,g.width*g.height*4-1,4 do argb[i+1],argb[i+3]=argb[i+3],argb[i+1] end -- swap red and blue

-- create a bmp in memory
	local bmp_data=pack.save_array(argb,"u8",0,#argb)
	local bmp_file=pack.save{2,"BM","u32",14+40+#bmp_data,"u16",0,"u16",0,"u32",14+40}
	local bmp_head=pack.save{"u32",40,"u32",g.width,"u32",g.height,"u16",1,"u16",32,"u32",0,"u32",#bmp_data,"u32",0,"u32",0,"u32",0,"u32",0}

-- create a bmp stream for SDL to read...
	local it={}
	it.dat=bmp_file..bmp_head..bmp_data
	it.off=0
	function it.size()
		return #it.dat
	end
	function it.close()
	end
	function it.seek(offset, whence)
		if whence == SDL.rwopsSeek.Set then
			it.off=offset
		elseif whence == SDL.rwopsSeek.Current then
			it.off=it.off+offset
		elseif whence == SDL.rwopsSeek.End then
			it.off=#it.dat-offset
		end
		if it.off<0 then it.off=0 end
		if it.off>#it.dat then it.off=#it.dat end
		return it.off
	end
	function it.read(n, size)
		local i=n*size
		if i>0 then
			local d=it.dat:sub(it.off+1,it.off+i)
			it.off=it.off+i
			return d,#d
		else
			return nil
		end
	end
	function it.write(data, n, size)
	end
	local rw=assert(SDL.RWCreate(it))
	local surf=assert(SDL.loadBMP_RW(rw))

	return surf
end

sdl.icon=function(w,g)

	w.win:setIcon(_grd_to_surface(g))

end


sdl.has_clipboard=function()
	return SDL.hasClipboardText()
end

sdl.get_clipboard=function()
	return SDL.getClipboardText()
end

sdl.set_clipboard=function(s)
	return SDL.setClipboardText(s)
end

-- we want lowercase only, and map to these capitals
local _cursor_names={
	"Arrow","Ibeam","Wait","Crosshair","WaitArrow",
	"SizeNWSE","SizeNESW","SizeWE","SizeNS","SizeAll",
	"No","Hand",
}


-- 24x24 is probably a safe and reasonable size?
--[=[
		wwin.cursor("test",[[
0 0 0 0 . . . . . . . . . . . . 0 0 0 0 . . . .
0 7 7 0 . . . . . . . . . . . . 0 7 7 0 . . . .
0 7 7 0 . . . . . . . . . . . . 0 7 7 0 . . . .
0 0 0 7 . . . . . . . . . . . . 0 0 0 7 . . . .
. . . . 7 . . . . . . . . . . . . . . . 7 . . .
. . . . . 7 . . . . . . . . . . . . . . . 7 . .
. . . . . . R R R R . . . . . . . . . . . . R R 
. . . . . . R R R R . . . . . . . . . . . . R R 
. . . . . . R R R R . . . . . . . . . . . . R R 
. . . . . . R R R R . . . . . . . . . . . . R R 
. . . . . . . . . . 7 . . . . . . . . . . . . . 
. . . . . . . . . . . 7 . . . . . . . . . . . . 
. . . . . . . . . . . . 7 0 0 0 . . . . . . . . 
. . . . . . . . . . . . 0 7 7 0 . . . . . . . . 
. . . . . . . . . . . . 0 7 7 0 . . . . . . . . 
. . . . . . . . . . . . 0 0 0 0 . . . . . . . . 
0 0 0 0 . . . . . . . . . . . . 0 0 0 0 . . . . 
0 7 7 0 . . . . . . . . . . . . 0 7 7 0 . . . . 
0 7 7 0 . . . . . . . . . . . . 0 7 7 0 . . . . 
0 0 0 7 . . . . . . . . . . . . 0 0 0 7 . . . . 
. . . . 7 . . . . . . . . . . . . . . . 7 . . . 
. . . . . 7 . . . . . . . . . . . . . . . 7 . . 
. . . . . . 7 . . . . . . . . . . . . . . . 7 . 
. . . . . . . 7 . . . . . . . . . . . . . . . 7 
]])
]=]

local _cursors={}
local _cursor=function(s,dat,px,py)

	if dat then -- remember a new one
			
		dat=_grd_to_surface(dat) -- convert from grd to surface
		
		_cursors[s]=assert(SDL.createColorCursor(dat , px or 0 , py or 0 ))
	
	end

	local v=_cursors[s]
	if v then return v end
	
	if type(s)~="string" then return end
	
	local S=s -- fix capitals
	for i,v in ipairs(_cursor_names) do
		if v:lower()==s then S=v break end
	end

	v=assert( SDL.createSystemCursor( SDL.systemCursor[S] ) )

	_cursors[s]=v

	return v
end

sdl.cursor=function(s,dat,px,py)

	local v=_cursor(s,dat,px,py)

	if not dat then -- if not setting an image then change the actual cursor
		if v then
			SDL.setCursor(v)
		else
			if type(s)=="nil" then -- default
				local v=_cursor("arrow")
				if v then SDL.setCursor(v) end
				SDL.showCursor(true)
			elseif type(s)=="boolean" then -- show/hide
				SDL.showCursor(s)
			end
		end
	end
	
end



sdl.platform=SDL.getPlatform() -- remember platform


return sdl

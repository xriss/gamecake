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
sdl.msg_fetch=function()

	for e in SDL.pollEvent() do

		if     	(e.type == SDL.event.KeyDown) or 
				(e.type == SDL.event.KeyUp) then

			local t={}
			t.time=sdl.time()
			t.class="key"
			t.ascii=""
			t.action=(e.type==SDL.event.KeyUp) and -1 or ( e["repeat"] and 0 or 1 )
			t.keycode=1
			t.keyname=SDL.getKeyName(e.keysym.sym)
			
--			dprint(t)

			sdl.queue[#sdl.queue+1]=t
					
		elseif     (e.type == SDL.event.TextInput) then

			local t=sdl.queue[#sdl.queue]
			if t and t.class=="key" and t.ascii=="" and e.text then -- insert ascii into last msg
				t.ascii=e.text
--				dprint(e)
			end
					
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

			local s=lookup(SDL.event,e.type)
			print(s.." : "..e.type)
			dprint(e)

		end
	end

end


sdl.warp_mouse=function(it,x,y)
	it=it or sdl.it
	it.win:warpMouse(x,y)
	return true
end



sdl.icon=function(w,g)

	local wgrd=require("wetgenes.grd")
	local pack=require("wetgenes.pack")

	assert(g:convert(wgrd.U8_RGBA)) -- make sure it is this format
	assert(g:flipy()) -- needs to be the other way up
	
	local argb=g:pixels(0,0,g.width,g.height)
	for i=0,g.width*g.height*4-1,4 do argb[i+1],argb[i+3]=argb[i+3],argb[i+1] end -- swap red and blue

-- create a bmp in memory
	local bmp_data=pack.save_array(argb,"u8",0,#argb)
	local bmp_file=pack.save{2,"BM","u32",14+40+#bmp_data,"u16",0,"u16",0,"u32",14+40}
	local bmp_head=pack.save{"u32",40,"u32",g.width,"u32",g.height,"u16",1,"u16",32,"u32",0,"u32",#bmp_data,"u32",0,"u32",0,"u32",0,"u32",0}

--print("ICON",#bmp_file,type(bmp_file),#bmp_head,type(bmp_file),#bmp_data,type(bmp_data))

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

	w.win:setIcon(surf)


--	core.rawicon(w,pack.save_array({g.width,g.height},"u32",0,2)..pack.save_array(argb,"u8",0,#argb))

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


sdl.platform=SDL.getPlatform() -- remember platform


return sdl

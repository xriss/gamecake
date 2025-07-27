--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")

local log,dump=require("wetgenes.logs"):export("log","dump")

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

sdl.hints=function(it)
	for n,v in pairs(it) do
		if string.sub(n,1,4)=="SDL_" then -- only SDL_ hints
			SDL.setHint(n,v,SDL.hintPriority.Normal)
		end
	end
end



sdl.print=print

sdl.video_init_done=false
sdl.video_init=function()
	if not sdl.video_init_done then
		sdl.video_init_done=true
		SDL.init{ SDL.flags.Video, SDL.flags.Joystick, SDL.flags.GameController, SDL.flags.Events, }
--		SDL.init{ SDL.flags.Everything, }
--		SDL.videoInit()

--[[
		local m=SDL.getDesktopDisplayMode(0)
		if m and m.h and m.w then
			m.w=1920
			m.h=1080
			local l=math.floor( ( math.log( math.sqrt((m.w*m.h)/(1920*1080)) ) / math.log(2) ) + 0.5 )
			local scale=math.pow(2,l) -- double size when we are within 50% of a 4k screen and so on upwards
			print(l,scale)
			sdl.suggested_scale=scale
		end
]]

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
			if ffi.C.FreeConsole then
				log( "oven","detaching from windows console")
				ffi.C.FreeConsole()
			end

		end)

	end

	local it={}
	sdl.it=it

	local flags={ SDL.window.Resizable , SDL.window.OpenGL }

	if jit and jit.os=="Windows" then -- windows does not do borderless resize, so, meh

	else
		if t.borderless then
			flags[#flags+1]=SDL.window.Borderless
		end
	end

	if t.hidden then -- allow hidden window for a fake headerless sort of thing
		flags[#flags+1]=SDL.window.Hidden
	end

--[[ -- dump modes
	do
		local num_video_displays=SDL.getNumVideoDisplays()
		for video_display_idx=0,num_video_displays-1 do
			local num_display_modes=SDL.getNumDisplayModes(video_display_idx)
			for display_mode_idx=0,num_display_modes-1 do
				local mode=SDL.getDisplayMode(video_display_idx,display_mode_idx)
				for n,v in pairs( SDL.pixelFormat ) do
					if v==mode.format then mode.format_name=n end
				end
				dump(mode)
			end
		end
	end
]]

	it.screen_mode=t.screen_mode -- try something like "640x480x60.RGB888"

	if it.screen_mode and ( type(it.screen_mode)~="table" ) then -- convert to mode table?

		it.screen_mode=tostring(it.screen_mode) -- allow a bool to just pick the default res which is 720p

		local mode={
			w=1280,
			h=720,
			refreshRate=0,
			format=SDL.pixelFormat.RGB888,
		}

--		print( it.screen_mode )
		local idx=1
		local names={"w","h","refreshRate"}
		for s in it.screen_mode:gmatch("([%u%d]+)") do
			local n=tonumber(s)
			if tostring(n)==s then -- got a number
				if names[idx] then -- got a place to put it
					mode[ names[idx] ]=n
					idx=idx+1
				end
			else
				n=SDL.pixelFormat[s]
				if n then -- got a pixel format
					mode.format=n
				end
			end
--			print(s,n)
        end
        it.screen_mode=mode

	end

--	dump("TRY")
--	dump(it.screen_mode)
--	it.screen_mode=nil


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

	if it.screen_mode then
		assert( it.win:setDisplayMode(it.screen_mode) )
	end

	if jit and jit.os=="Windows" then -- windows does not do borderless resize, so, meh
	else
		if t.borderless then
			require("wetgenes.win.core").sdl_attach_resize_hax(it.win)
		end
	end

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

	local screen=wwin.screen()
	local x=math.floor((screen.width-w)*(0.5))
	local y=math.floor((screen.height-h)*(0.5))
	if x<0 then x=0 end
	if y<0 then y=0 end
	if it and it.win then it.win:setPosition(x,y) end

end

sdl.show=function(it,view)
--	print("SDL show")
	it=it or sdl.it
	it.win:show()

--  it.win:setResizeable(false) it.win:setBordered(false)

	if     view=="full" then
		if it.screen_mode then -- we want to force this fullscreen mode
			it.win:setFullscreen(0)
			it.win:restore()
			it.win:setSize( it.screen_mode.w , it.screen_mode.h )
			it.win:setDisplayMode(it.screen_mode)
			it.win:setFullscreen(SDL.window.Fullscreen)
		else
			it.win:setFullscreen(SDL.window.Desktop)
		end
	elseif view=="max"  then	 it.win:maximize()
	else						 it.win:setFullscreen(0) it.win:restore()
	end

--	sdl.mousexy_init()

	return nil
end


sdl.context=function(it)

--	print("SDL context")
	it=it or sdl.it

--[[

	if string.find(SDL.getPlatform(),"Mac") then

-- OSX requires some fluffing to get a working GL context

print("SDL detected OSX : "..SDL.getPlatform())

		SDL.glSetAttribute(SDL.glAttr.ContextProfileMask,SDL.glProfile.Core)
		SDL.glSetAttribute(SDL.glAttr.ContextFlags,SDL.glFlags.ForwardCompatible)
		SDL.glSetAttribute(SDL.glAttr.ContextMajorVersion,3)
		SDL.glSetAttribute(SDL.glAttr.ContextMinorVersion,2)

	elseif string.find(SDL.getPlatform(),"Emscripten") then

print("SDL detected EMCC : "..SDL.getPlatform())
--ask emscripten for a webgl 2.0 / es 3.0 context

		SDL.glSetAttribute(SDL.glAttr.ContextMajorVersion,3)

	end

]]

	if not it.ctx then -- request an opengl 3.2 core context

		SDL.glSetAttribute(SDL.glAttr.ContextProfileMask,SDL.glProfile.Core)
		SDL.glSetAttribute(SDL.glAttr.ContextMajorVersion,3)
		SDL.glSetAttribute(SDL.glAttr.ContextMinorVersion,2)

		it.ctx=SDL.glCreateContext(it.win)

	end

	if not it.ctx then -- request an opengl es 3.0 context

		SDL.glSetAttribute(SDL.glAttr.ContextProfileMask, SDL.glProfile.ES)
		SDL.glSetAttribute(SDL.glAttr.ContextMajorVersion, 3)
		SDL.glSetAttribute(SDL.glAttr.ContextMinorVersion, 0)

		it.ctx=SDL.glCreateContext(it.win)

	end

	if not it.ctx then -- request an opengl es 2.0 context

		SDL.glSetAttribute(SDL.glAttr.ContextProfileMask, SDL.glProfile.ES)
		SDL.glSetAttribute(SDL.glAttr.ContextMajorVersion, 2)
		SDL.glSetAttribute(SDL.glAttr.ContextMinorVersion, 0)

		it.ctx=SDL.glCreateContext(it.win)

	end


	assert(it.ctx)

	SDL.glMakeCurrent(it.win, it.ctx)
	SDL.glSetSwapInterval(1) -- enable vsync by default


	local gles=require("gles")
	log( "oven","vendor",(gles.Get(gles.VENDOR) or ""))
	log( "oven","render",(gles.Get(gles.RENDERER) or ""))
	log( "oven","version",(gles.Get(gles.VERSION) or ""))

-- this is depreciated
--	for w in (gles.Get(gles.EXTENSIONS) or ""):gmatch("([^%s]+)") do
--		log( "oven","glext",w)
--	end
-- need to do this
	do
		local t={}
		for i=0,(gles.Get(gles.NUM_EXTENSIONS) or 0)-1 do
			t[#t+1]=gles.Get(gles.EXTENSIONS,i)
		end
		table.sort(t)
		for i,s in ipairs(t) do log( "oven","glext",s) end
	end

	return it.ctx
end

sdl.swap=function(it)
	it=it or sdl.it
	SDL.glSwapWindow(it.win)
end


sdl.sleep=function(sec)
--	print("SDL sleep")
	SDL.delay(sec*1000) -- convert seconds to milliseconds
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
	log("sdl","wait")
	return nil
end

sdl.mousexy={-1,-1}

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

			local t={}
			t.time=sdl.time()
			t.class="mouse"
			t.action=0
			t.x=sdl.mousexy[1]
			t.y=sdl.mousexy[2]
			t.dx=0
			t.dy=0

			if 		e.y>0 then	t.keycode=4 t.action=-1
			elseif 	e.y<0 then	t.keycode=5 t.action=-1
			elseif 	e.x>0 then	t.keycode=6 t.action=-1
			elseif 	e.x<0 then	t.keycode=7 t.action=-1 end

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

			t.dx=e.xrel or 0
			t.dy=e.yrel or 0

			sdl.queue[#sdl.queue+1]=t

			sdl.mousexy[1]=e.x
			sdl.mousexy[2]=e.y

			if not sdl.relative_mouse_mode then
				if SDL.captureMouse then -- sanity test
					if     e.type == SDL.event.MouseButtonDown then SDL.captureMouse(true)
					elseif e.type == SDL.event.MouseButtonUp   then SDL.captureMouse(false)
					end
				end
			end

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


		elseif	(e.type == SDL.event.WindowEvent) then -- window related, eg focus resize etc

			if ( e.event == SDL.eventWindow.SizeChanged ) then

				local t={}
				t.time=sdl.time()
				t.class="resize"

				sdl.queue[#sdl.queue+1]=t

			end

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


--			local s=lookup(SDL.event,e.type)
--			print(s.." : "..e.type)
--			dprint(e)

		end
	end

end

sdl.relative_mouse_mode=false
sdl.relative_mouse=function(it,mode)
	local oldmode=sdl.relative_mouse_mode
	sdl.relative_mouse_mode=mode
	SDL.setRelativeMouseMode(mode)
	return oldmode
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


-- show/hide onscreen keyboard
function sdl.StartTextInput(...)
	return SDL.startTextInput(...)
end
function sdl.StopTextInput(...)
	return SDL.stopTextInput(...)
end
function sdl.GetPrefPath(...)
	return SDL.getPrefPath(...)
end

sdl.platform=SDL.getPlatform() -- remember platform

return sdl

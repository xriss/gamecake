--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local sdl={}

local SDL=require("SDL")

local wstr=require("wetgenes.string")
local dprint=function(a) print(wstr.dump(a)) end

sdl.print=print

sdl.video_init_done=false
sdl.video_init=function()
	if not sdl.video_init_done then
		sdl.video_init_done=true
		SDL.videoInit()
	end
end

sdl.screen=function()
	print("SDL screen")
	sdl.video_init()
	local b=SDL.getDisplayBounds(0)
	return b.w,b.h
end


sdl.create=function(t)
	print("SDL create")

--[[
	SDL.glSetAttribute(SDL.glAttr.RedSize, 1);
	SDL.glSetAttribute(SDL.glAttr.GreenSize, 1);
	SDL.glSetAttribute(SDL.glAttr.BlueSize, 1);
	SDL.glSetAttribute(SDL.glAttr.DepthSize, 1);
	SDL.glSetAttribute(SDL.glAttr.DoubleBuffer, 1);
]]

	local it={}
	it.win= assert(SDL.createWindow {
		title   = t.title,
		width   = t.width,
		height  = t.height,
		flags   = { SDL.window.Resizable , SDL.window.OpenGL },
		x       = t.x,
		y       = t.y,
	})

	return it
end

sdl.destroy=function(it)
	print("SDL destroy")
	return nil
end

sdl.info=function(it,w)
--	print("SDL info")
	w=w or {}
	w.width,w.height=it.win:getSize()
end

sdl.show=function(it)
	print("SDL show")
	it.win:show()
	return nil
end

sdl.context=function(it)
	print("SDL context")
	it.ctx=assert(SDL.glCreateContext(it.win))
	SDL.glMakeCurrent(it.win, it.ctx)
	return it.ctx
end

sdl.swap=function(it)
--	print("SDL swap")
	SDL.glSwapWindow(it.win)
end
	

sdl.sleep=function()
--	print("SDL sleep")
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
	print("SDL peek")
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
	
		
		
		else
--			dprint(e)
		end
	end
	
--[[
		if e.type == SDL.event.Quit then
								running = false
		elseif e.type == SDL.event.KeyDown then
		print(string.format("key down: %d -> %s", e.keysym.sym, SDL.getKeyName(e.keysym.sym)))
		elseif e.type == SDL.event.MouseWheel then
		print(string.format("mouse wheel: %d, x=%d, y=%d", e.which, e.x, e.y))
		elseif e.type == SDL.event.MouseButtonDown then
		print(string.format("mouse button down: %d, x=%d, y=%d", e.button, e.x, e.y))
		elseif e.type == SDL.event.MouseMotion then
		print(string.format("mouse motion: x=%d, y=%d", e.x, e.y))
		end
]]

end


return sdl

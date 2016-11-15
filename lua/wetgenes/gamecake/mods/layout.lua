--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,layout)

	layout=layout or {}

-- "main" , "main+chat" , "main+chat+keys" ,  "main+keys" 
	layout.mode="main"
--	layout.mode="main+chat+keys"

-- info about some of the areas we offer

	local cake=oven.cake
	local layouts=cake.layouts


	layout.main=layouts.create{}  -- where you should put your main view
	layout.keys=layouts.create{}  -- a place to type on devices without a keyboard
	layout.chat=layouts.create{}  -- a place to view what other people type

	function layout.cycle_mode()
		if layout.mode=="main" then
			if oven.mods["wetgenes.gamecake.mods.chat"] then
				layout.mode="main+chat"
			elseif oven.mods["wetgenes.gamecake.mods.keys"] then
				layout.mode="main+keys"
			end
		elseif layout.mode=="main+chat" then
			if oven.mods["wetgenes.gamecake.mods.keys"] then
				layout.mode="main+chat+keys"
			else
				layout.mode="main"
			end
		elseif layout.mode=="main+chat+keys" then
			layout.mode="main+keys"
		else
			layout.mode="main"
		end
	end

	function layout.setup()

	end

	function layout.clean()
	
	end
	
	

	function layout.update()

		oven.win:info()
		
		local w=oven.win.width
		local h=oven.win.height
				
		layout.main.active=false
		layout.keys.active=false
		layout.chat.active=false
		
		local port=true -- portrait mode?
		if w>=h then port=false end -- landscape mode

		if			layout.mode=="main" then
		
			layout.main.active=true
			layout.main.x=0
			layout.main.y=0
			layout.main.w=w
			layout.main.h=h

		elseif		layout.mode=="main+chat" then
		
			if port then
			
				layout.chat.active=true
				layout.chat.x=0
				layout.chat.y=0
				layout.chat.w=w
				layout.chat.h=math.floor(h/2)
				
				layout.main.active=true
				layout.main.x=0
				layout.main.y=layout.chat.h
				layout.main.w=w
				layout.main.h=h-layout.chat.h

			else
			
				layout.main.active=true
				layout.main.x=0
				layout.main.y=0
				layout.main.w=math.floor(w*2/3)
				layout.main.h=h
				
				layout.chat.active=true
				layout.chat.x=layout.main.w
				layout.chat.y=0
				layout.chat.w=w-layout.main.w
				layout.chat.h=h

			end
		
		elseif		layout.mode=="main+chat+keys" then

			if port then
			
				layout.chat.active=true
				layout.chat.x=0
				layout.chat.y=0
				layout.chat.w=w
				layout.chat.h=math.floor(h/3)
				
				layout.main.active=true
				layout.main.x=0
				layout.main.y=layout.chat.h
				layout.main.w=w
				layout.main.h=math.floor(h/3)

				layout.keys.active=true
				layout.keys.x=0
				layout.keys.y=layout.chat.h + layout.main.h
				layout.keys.w=w
				layout.keys.h=h - layout.main.h - layout.chat.h

			else
			
				layout.main.active=true
				layout.main.x=0
				layout.main.y=0
				layout.main.w=math.floor(w*2/3)
				layout.main.h=math.floor(h*2/3)
				
				layout.chat.active=true
				layout.chat.x=layout.main.w
				layout.chat.y=0
				layout.chat.w=w-layout.main.w
				layout.chat.h=h

				layout.keys.active=true
				layout.keys.x=0
				layout.keys.y=layout.main.h
				layout.keys.w=layout.main.w
				layout.keys.h=h - layout.main.h

			end
		
		elseif		layout.mode=="main+keys" then
			
			layout.main.active=true
			layout.main.x=0
			layout.main.y=0
			layout.main.w=w
			layout.main.h=math.floor(h*2/3)
			
			layout.keys.active=true
			layout.keys.x=0
			layout.keys.y=layout.main.h
			layout.keys.w=w
			layout.keys.h=h-layout.main.h
			
		end
				
	end
	
	function layout.draw() -- we dont need to draw but we will update again
	
		layout.update()
		
	end
		
--	function layout.msg(m)
--		return m
--	end

	return layout
end

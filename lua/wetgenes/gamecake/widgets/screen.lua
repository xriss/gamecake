--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[

a screen is a window container, windows added to a screen will be 
constrained to within its bounds, a window can also contain a screen 
that contains other windows. So it is a window grouping system.

Screens provide a background window layer that allow windows to snap 
into edges and exist as tiles without overlap, as in a tiling window 
manager.

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wscreen)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wscreen=wscreen or {}

function wscreen.update(widget)
	return widget.meta.update(widget)
end

function wscreen.draw(widget)
	return widget.meta.draw(widget)
end

function wscreen.layout(widget)
	return widget.meta.layout(widget)
end

-- create a new top level screen split to snap windows into
function wscreen.get_split(screen)
	for i,v in ipairs(screen) do
		if v.class=="split" then return v end
	end
end

-- create a new top level screen split to snap windows into
function wscreen.add_split(screen,opts)

	local old_split=wscreen.get_split(screen)

--print("OLD",old_split,old_split and old_split.class,old_split and old_split.id)

	local def={class="split",size="full"}
	for n,v in pairs(opts) do
		if type(n)=="string" then
			if string.sub(n,1,6)=="split_" then -- copy all split_ members
				def[n]=v
			end
		end
	end
	local split=screen:add(def)
	screen:insert(split,1) -- move to first place 

	if old_split then
		split:insert(old_split)
	else
		split:insert(screen.windows)
	end

	local dock=split:add({class="windock",windock="panel",color=1})
	if opts.window then
		dock:insert(opts.window)
		if not split.split_max then
			if split.split_axis=="x" then
				split.split_max=opts.window.win_fbo.hx
			else
				split.split_max=opts.window.win_fbo.hy
			end
		end
	end
	if split.split_order~=2 then
		local t=split[2]
		split[2]=split[1]
		split[1]=t
	end
	
	split:set_dirty()	
	screen.master:layout()
	screen.master:build_m4()

end

-- create a new top level screen split to snap windows into
function wscreen.remove_split(screen,window)

	local dock=window.parent
	local split=dock.parent

	local parent=split.parent
	
--print("PARENT",parent,parent.class,parent.id)
	local other=(split[1]==dock) and split[2] or split[1]
--print("S",#split)
	parent:insert(other) -- keep the other half
	split:remove()
--print("P",#parent)
	
	screen.windows:insert(window)
	
	screen.master:layout()
	screen.master:build_m4()
end


function wscreen.setup(widget,def)

	widget.class="screen"
	
	widget.update=wscreen.update
	widget.draw=wscreen.draw
	widget.layout=wscreen.layout

	widget.get_split=wscreen.get_split
	widget.add_split=wscreen.add_split
	widget.remove_split=wscreen.remove_split

	widget.windows=widget:add(def.windows or {class="windock",windock="drag",size="full"})
	
	return widget
end

return wscreen
end

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
local wwindow=oven.rebake("wetgenes.gamecake.widgets.window")

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

-- find all windows in this screen
function wscreen.get_windows(screen)
	local windows={}
	screen:call_descendents(function(w)
		if w.class=="window" then
			windows[#windows+1]=w
		end
	end)
	return windows
end

wscreen.window_menu=function(screen)
	local windows=screen:get_windows()
	table.sort(windows,function(a,b) return a.win_title.text < b.win_title.text end)
	local t={hooks=function(act,w) return wwindow.window_hooks(screen,act,w) end}
	for i,v in ipairs(windows) do
		t[#t+1]={id="win_toggle_other",user=v.id,text=v.win_title.text}
	end
	return t
end


-- create a new top level screen split to snap windows into
function wscreen.get_split(screen,axis,order)

	if not axis then
		for i,v in ipairs(screen) do
			if v.class=="split" then return v end
		end
	end
	
	for i,v in ipairs(screen) do
		if v.class=="split" then
			if v.split_axis==axis and v.split_order==order then return v end
			local ret=wscreen.get_split(v,axis,order) -- recurse
			if ret then return ret end
		end
	end
	
end

-- create a new top level screen split to snap windows into
function wscreen.add_split(screen,opts)

	local old_split=wscreen.get_split(screen) or screen.windows

--print("OLD",old_split,old_split and old_split.class,old_split and old_split.id)

	local def={class="split",size="full"}
	for n,v in pairs(opts) do
		if type(n)=="string" then
			if string.sub(n,1,6)=="split_" then -- copy all split_ members
				def[n]=v
			end
		end
	end
	
	local split
	
	if opts.internal then -- split inside
	
		local idx=screen.windows:parent_index()

		split=screen.windows.parent:add(def)
		split.screen_split=true

		screen.windows.parent:insert(split,idx) -- move next to windows
		split:insert(screen.windows)
			
	else -- split outside

		split=screen:add(def)
		split.screen_split=true

		screen:insert(split,1) -- move to first place 
		split:insert(old_split)

	end


	local dock=split:add({class="windock",windock="stack",stack_axis=(split.split_axis=="x")and"y"or"x"})
	if opts.window then
		dock:insert(opts.window)
		if not split.split_num then
			if split.split_axis=="x" then
				split.split_num=opts.window.win_fbo.hx
				if opts.window.win_fbo.hy > split.parent.hy then
					local s=split.parent.hy / opts.window.win_fbo.hy
					split.split_num=opts.window.win_fbo.hx*s
				end
			else
				split.split_num=opts.window.win_fbo.hy
				if opts.window.win_fbo.hx > split.parent.hx then
					local s=split.parent.hx / opts.window.win_fbo.hx
					split.split_num=opts.window.win_fbo.hy*s
				end
			end
		end
		if not split.split_fmax then split.split_fmax = 7/16 end
		if not split.split_fmin then split.split_fmin = 1/16 end
	end
	if split.split_order~=2 then -- move to the other side
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

	if #dock==1 then -- remove split only when last window is removed from dock
	
		local other=(split[1]==dock) and split[2] or split[1]
		local num=1
		for i,v in ipairs(parent) do if v==split then num=i end end
		parent:insert(other,num) -- put the other half where the split was
		split:remove() -- and remove the split

	end
	
	screen.windows:insert(window)
	
	screen.master:layout()
	screen.master:build_m4()
end


function wscreen.setup(widget,def)

	widget.class="screen"
	
	widget.prefab={} -- prefab widget constructors
	
	widget.update=wscreen.update
	widget.draw=wscreen.draw
	widget.layout=wscreen.layout

	widget.get_windows=wscreen.get_windows
	widget.window_menu=wscreen.window_menu

	widget.get_split=wscreen.get_split
	widget.add_split=wscreen.add_split
	widget.remove_split=wscreen.remove_split

	widget.windows=widget:add(def.windows or {class="windock",windock="drag",size="full"})
	
	return widget
end

return wscreen
end

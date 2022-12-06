--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.widgets.dialogs

handle a collection of dialogs that all live in the same place

]]

-- module bake
local M={ modname = (...) } package.loaded[M.modname] = M function M.bake(oven,B) B = B or {}

--local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")
--local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

local wdialogs=B


--[[#lua.wetgenes.gamecake.widgets.dialogs.show_overlay

]]
wdialogs.show_overlay=function(dialogs)
	local screen=dialogs.parent
	dialogs.overlay=screen:add{size="full",solid=true,transparent=0xcc000000,class="center"}
	return dialogs.overlay
end

--[[#lua.wetgenes.gamecake.widgets.dialogs.hide_overlay

]]
wdialogs.hide_overlay=function(dialogs)
	dialogs.overlay:remove() -- hide/delete all of our widgets
	dialogs.overlay=nil
end


--[[#lua.wetgenes.gamecake.widgets.dialogs.show

]]
wdialogs.show=function(dialogs,opts)

	local master=dialogs.master
	local screen=dialogs.parent

	local hz=master.theme.grid_size


	local def_window=function(parent,it)
		for n,v in pairs{
			class="window",
			hx=hz*16,
			hy=hz*16,
			px=0,
			py=0,
			solid=true,
			flags={nobar=true,nosort=true},
		} do it[n]=it[n] or v end
		
		return parent:add(it)
	end

	local def_button=function(parent,it)
		for n,v in pairs{
			hx=hz*8,
			hy=hz,
			px=2,
			py=2,
			color=0,
			solid=true,
			hooks=opts.hooks,
		} do it[n]=it[n] or v end
		return parent:add_border(it)
	end

	dialogs:show_overlay() -- dim screen with an overlay

	local window=def_window(dialogs.overlay,{px=screen.hx/2,py=screen.hy/2,hx=hz*16.5,size="fity",id="request",title=""})
	local canvas=window.win_canvas
	
	window.close_request=function(id)
		dialogs:hide_overlay()
		master:layout() -- need to layout at least once to get everything in the right place
		if id and opts[id] then (opts[id])(window) end
	end

	canvas:add({hx=hz*16.5,hy=hz*0.25})

	canvas:add({hx=hz*0.25,hy=hz*0.25})
	local inside=canvas:add({hx=hz*16,size="fity",class="fill"})
	canvas:add({hx=hz*0.25,hy=hz*0.25})

	for i,v in ipairs(opts.lines or {} ) do

		local it={
		
			hx=hz*16,
			hy=hz,
			px=2,
			py=2,
			text=v
		}
		inside:add_border(it)
	end
	
	
	if opts.file then
		window.file = inside:add({hx=hz*16,hy=hz*16,class="file",id="file",hooks=opts.hooks})
	end


	local clickhooks=function(act,w,dat)
		if act=="click" then
			window.close_request(w.id)
		end
	end
	if opts.no then
		def_button(inside,{hooks=opts.hooks or clickhooks,class="button",id="no",text="No"})
	end
	if opts.cancel then
		def_button(inside,{hooks=opts.hooks or clickhooks,class="button",id="cancel",text="Cancel"})
	end
	if opts.ok then
		def_button(inside,{hooks=opts.hooks or clickhooks,class="button",id="ok",text="OK"})
	end
	if opts.yes then
		def_button(inside,{hooks=opts.hooks or clickhooks,class="button",id="yes",text="Yes"})
	end
	if opts.sorry then
		def_button(inside,{hooks=opts.hooks or clickhooks,class="button",id="sorry",text="Sorry",hx=hz*16})
	end

	canvas:add({hx=hz*16.5,hy=hz*0.25})

	master:resize_and_layout() -- need to layout at least once to get everything in the right place

-- layout twice still neded? not sure why but will look into it, untill then...
	master:layout() -- need to layout at least once to get everything in the right place

end


--[[#lua.wetgenes.gamecake.widgets.dialogs.setup

]]
function wdialogs.setup(widget,def)

	widget.class="dialogs"
	
	widget.smode=widget.smode or "topleft"
	
	
-- copy all wdialogs functions
	for n,v in pairs(wdialogs) do
		if type(v)=="function" then
			if not widget[n] then
				widget[n]=v
			end
		end
	end

	return widget
end

return wdialogs
end




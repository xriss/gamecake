--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- handle a collection of dialogs that all live in the same place

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wdialogs)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")
local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

wdialogs=wdialogs or {}

function wdialogs.update(widget)
	return widget.meta.update(widget)
end

function wdialogs.draw(widget)
	return widget.meta.draw(widget)
end

function wdialogs.layout(widget)
	return widget.meta.layout(widget)
end

wdialogs.show_overlay=function(dialogs)
	local screen=dialogs.parent
	dialogs.overlay=screen:add{class="center",solid=true,transparent=0xcc000000}
	return dialogs.overlay
end

wdialogs.hide_overlay=function(dialogs)
	dialogs.overlay:remove() -- hide/delete all of our widgets
	dialogs.overlay=nil
end


wdialogs.show=function(dialogs,opts)

	local hz=master.grid_size or 24
	local master=dialogs.master
	local screen=dialogs.parent

	local def_window=function(parent,it)
		for n,v in pairs{
			class="window",
			hx=hz*10,
			hy=hz*10,
			px=0,
			py=0,
			solid=true,
			flags={nodrag=true,nobar=true},
		} do it[n]=it[n] or v end
		
		return parent:add(it)
	end

	local def_button=function(parent,it)
		for n,v in pairs{
			hx=hz*5,
			hy=hz,
			px=2,
			py=2,
			color=0,
			solid=true,
			hooks=dialogs.hooks,
		} do it[n]=it[n] or v end
		return parent:add_border(it)
	end

	
	dialogs:show_overlay() -- dim screen with an overlay

	local window=def_window(dialogs.overlay,{px=screen.hx/2,py=screen.hy/2,hx=hz*10,size="fit",id="request",title=""})
	local canvas=window.win_canvas
	
	window.close_request=function(id)
		dialogs:hide_overlay()
		master:layout() -- need to layout at least once to get everything in the right place
		if opts[id] then (opts[id])() end
	end

	for i,v in ipairs(opts.lines or {} ) do

		local it={
		
			hx=hz*10,
			hy=hz,
			px=2,
			py=2,
			text=v
		}
		canvas:add_border(it)
	end
	
	local hooks=function(act,w,dat)
	
		if act=="click" then

			window.close_request(w.id)

		end
	
	end
	
	if opts.sorry then
		def_button(canvas,{hooks=hooks,class="button",id="sorry",text="Sorry",hx=hz*10})
	end

	if opts.yes then
		def_button(canvas,{hooks=hooks,class="button",id="yes",text="Yes"})
	end
	if opts.ok then
		def_button(canvas,{hooks=hooks,class="button",id="ok",text="OK"})
	end
	if opts.no then
		def_button(canvas,{hooks=hooks,class="button",id="no",text="No"})
	end

	master:resize_and_layout() -- need to layout at least once to get everything in the right place

-- layout twice still neded? not sure why but will look into it, untill then...
	master:layout() -- need to layout at least once to get everything in the right place

end


function wdialogs.setup(widget,def)

	widget.class="dialogs"
	
	widget.smode=def.smode or "topleft"
	
	
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




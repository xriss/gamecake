--
-- (C) 2020 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local function print(...) _G.print(...) end
local function dprint(a) print(require("wetgenes.string").dump(a)) end

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wpath=require("wetgenes.path")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(oven,wtree)

	wtree=wtree or {} 
	wtree.modname=M.modname
	

wtree.refresh=function(widget,items)

	if items then widget.items=items end
	
	local pan=widget.scroll_widget.pan
	pan:remove_all()

	local ss=widget.master.theme.grid_size

	local recurse ; recurse=function(items,depth)
	
		for i,it in ipairs(items) do
		
			local opts={
				class="button",
				id=it.id or widget.id,
				class_hooks=it.class_hooks or widget.class_hooks,
				hooks=it.hooks or widget.hooks,
				hx=ss,
				hy=ss,
				size="fullx",
				text_align="left",
				user=it,
				color=0,
			}
			if it.refresh then
				it.refresh(it,depth,opts)
			end
			if not opts.text then
				opts.text=it.text and string.rep("  ",depth)..it.text
			end

			pan:add(opts)

			if it[1] then -- children
				recurse(it,depth+1)
			end
		
		end
		
	end ; recurse(widget.items,0)
	
	widget.master.request_layout=true
end


function wtree.setup(widget,def)

	widget.class="tree"

	widget.items={} -- items to display
	widget.refresh=wtree.refresh -- rebuild display from items
--	widget.tree_hooks=wtree.tree_hooks -- handle clicks to expand or collapse

	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,size="full",class="scroll"})

	widget:refresh()

	return widget
end


	return wtree
end

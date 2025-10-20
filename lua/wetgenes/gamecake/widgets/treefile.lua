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


M.bake=function(oven,wtreefile)

	wtreefile=wtreefile or {} 
	wtreefile.modname=M.modname
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")


function wtreefile.refresh(treefile)
	
	treefile.tree_widget:refresh()

end



function wtreefile.setup(widget,def)

	widget.class="treefile"

	widget.add_dir_item=wtreefile.add_dir_item
	widget.add_file_item=wtreefile.add_file_item

	widget.refresh=wtreefile.refresh
	
	local ss=widget.master.theme.grid_size

	widget.data_dir  = widget.data_dir  or wdata.new_data({class="string",str=wpath.currentdir(),master=widget.master,hooks=wtreefile.class_hooks})

	widget.tree_widget=widget:add({size="full",class="tree",id="gists"})
	widget.tree_widget.items=require("wetgenes.gamecake.widgets.mount").mount_file()
	widget.tree_widget.items:manifest_path( widget.data_dir:value() )
	widget.tree_widget.hooks=function(...) return widget.hooks(...) end -- hook up sub widget

	return widget
end


	return wtreefile
end

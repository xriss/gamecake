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


M.bake=function(oven,wtreegist)

	wtreegist=wtreegist or {} 
	wtreegist.modname=M.modname
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")


function wtreegist.refresh(treefile)
	
	treefile.tree_widget:refresh()

end



function wtreegist.setup(widget,def)

	widget.class="treegist"

	widget.add_dir_item=wtreegist.add_dir_item
	widget.add_file_item=wtreegist.add_file_item

	widget.refresh=wtreegist.refresh
	
	local ss=widget.master.theme.grid_size

	widget.data_dir  = widget.data_dir  or wdata.new_data({class="string",str=wpath.currentdir(),master=widget.master,hooks=wtreegist.class_hooks})

	widget.tree_widget=widget:add({size="full",class="tree",id="gists"})
	widget.tree_widget.items=require("wetgenes.gamecake.widgets.datatree").mount_fs()
	widget.tree_widget.items:manifest_path( widget.data_dir:value() )
	widget.tree_widget.hooks=function(...) return widget.hooks(...) end -- hook up sub widget

	return widget
end


	return wtreegist
end

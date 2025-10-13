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

local _,lfs=pcall( function() return require("lfs") end ) ; lfs=_ and lfs

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


function wtreegist.class_hooks(hook,widget,dat)

	if hook=="unfocus" or hook=="timedelay" then
		if widget.id=="dir" then
			local treefile=widget
			while treefile and treefile.parent~=treefile and treefile.class~="treefile" do treefile=treefile.parent end
			if treefile.class=="treefile" then -- sanity
				treefile:refresh()
			end
		end
	end
	
	if hook=="click" and widget and widget.id=="file" then
		local it=widget.user
		local tree=widget ; while tree and tree.parent~=tree and tree.class~="tree" do tree=tree.parent end
		local treefile=tree.parent
		if treefile.class=="treefile" then -- sanity
			if it.mode=="directory" then

--				treefile:item_toggle_dir(it)

				if it.refresh then it:refresh() end

				tree:refresh()

			end

			treefile:call_hook_later("line_click",it)

		end
	end
end


function wtreegist.setup(widget,def)

	widget.class="treegist"

	widget.add_dir_item=wtreegist.add_dir_item
	widget.add_file_item=wtreegist.add_file_item
	
--	widget.item_refresh    = widget.item_refresh    or wtreegist.item_refresh
--	widget.item_toggle_dir = widget.item_toggle_dir or wtreegist.item_toggle_dir
--	widget.item_empty_dir  = widget.item_empty_dir  or wtreegist.item_empty_dir
--	widget.item_fill_dir   = widget.item_fill_dir   or wtreegist.item_fill_dir
--	widget.item_sort       = widget.item_sort       or wtreegist.item_sort

	widget.refresh=wtreegist.refresh
	widget.class_hooks={wtreegist.class_hooks}
	
	local ss=widget.master.theme.grid_size

	widget.data_dir  = widget.data_dir  or wdata.new_data({class="string",str=wpath.currentdir(),master=widget.master,hooks=wtreegist.class_hooks})

--	widget.split_widget=widget:add({size="full",class="split",split_axis="y",split_order=1})
--	widget.dir_widget=widget.split_widget:add({hy=ss,class="textedit",color=0,data=widget.data_dir,clip2=true,hooks=wtreegist.class_hooks,id="dir"})
--	widget.tree_widget=widget.split_widget:add({class="tree",id="files"})

	widget.tree_widget=widget:add({size="full",class="tree",id="gists"})
	widget.tree_widget.items=require("wetgenes.gamecake.widgets.datatree").mount_fs()
	widget.tree_widget.items:manifest_path( widget.data_dir:value() )

	widget:refresh()

	return widget
end


	return wtreegist
end

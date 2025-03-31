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


M.bake=function(oven,wtreefile)

	wtreefile=wtreefile or {} 
	wtreefile.modname=M.modname
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")
	

function wtreefile.refresh_items(widget,items)

	local files={}
	pcall( function()
		for n in lfs.dir(items.path) do
			if n~="." and n~=".." then
				local t=lfs.attributes(wpath.resolve(items.path,n))
				if t then
					t.name=n
					t.path=wpath.resolve(items.path,t.name)
					t.prefix=""
					files[#files+1]=t
				end
			end
		end
	end)
	
	table.sort(files,function(a,b)
		if a.mode == b.mode then
			return b.name:lower() > a.name:lower()
		else
			return b.mode > a.mode
		end
	end)

	for i,v in ipairs(files) do
		local it={}

		it.mode=v.mode
		it.path=v.path
		it.name=v.name
		it.depth=items.depth+1

		if it.mode=="directory" then
			it.text=string.rep("  ",it.depth-1)..it.name.."/"
		else
			it.text=string.rep("  ",it.depth-1)..it.name
		end

		if widget.refresh_item then
			it.refresh=widget.refresh_item
		end
		if it.refresh then it:refresh() end

		items[#items+1]=it
		
	end

end

function wtreefile.refresh(treefile)

--	if not treefile.tree_widget.items.path then

		local items={}
		
		items[1]={
			class="textedit",
			color=0,
			data=treefile.data_dir,
			clip2=true,
			hooks=wtreefile.class_hooks,
			id="dir",
		}

		items.path=wpath.resolve(treefile.data_dir:value())
		items.mode="directory"
		items.depth=0
		wtreefile.refresh_items(treefile,items)

		treefile.tree_widget:refresh(items)

--	end

	treefile.tree_widget:refresh()
end

function wtreefile.class_hooks(hook,widget,dat)

	if hook=="unfocus_edit" or hook=="timedelay" then
		if widget.id=="dir" then
			local treefile=widget
			while treefile and treefile.parent~=treefile and treefile.class~="treefile" do treefile=treefile.parent end
			if treefile.class=="treefile" then -- sanity
				treefile:refresh()
			end
		end
	end
	
	if hook=="click" and widget and widget.id=="files" then
		local it=widget.user
		local tree=widget ; while tree and tree.parent~=tree and tree.class~="tree" do tree=tree.parent end
		local treefile=tree.parent
		if treefile.class=="treefile" then -- sanity
			if it.mode=="directory" then

				if it[1] then -- hide dir
					for i=#it,1,-1 do
						it[i]=nil
					end
				else -- show dir
					wtreefile.refresh_items(treefile,it)
				end
				
				if it.refresh then it:refresh() end

				tree:refresh()

			elseif it.mode=="file" then
			
				treefile:call_hook_later("file_name_click",it)

			end

		end
	end
end


function wtreefile.setup(widget,def)

	widget.class="treefile"

	widget.refresh=wtreefile.refresh
	widget.class_hooks={wtreefile.class_hooks}

	local ss=widget.master.theme.grid_size

	widget.data_dir  = widget.data_dir  or wdata.new_data({class="string",str=wpath.currentdir(),master=widget.master,hooks=wtreefile.class_hooks})

--	widget.split_widget=widget:add({size="full",class="split",split_axis="y",split_order=1})
--	widget.dir_widget=widget.split_widget:add({hy=ss,class="textedit",color=0,data=widget.data_dir,clip2=true,hooks=wtreefile.class_hooks,id="dir"})
--	widget.tree_widget=widget.split_widget:add({class="tree",id="files"})

	widget.tree_widget=widget:add({size="full",class="tree",id="files"})	
	widget.tree_widget.class_hooks={wtreefile.class_hooks}

	widget:refresh()

	return widget
end


	return wtreefile
end

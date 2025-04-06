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

function wtreefile.refresh_item(item,widget)

	local ss=widget.master.theme.grid_size		
	local opts={
		class="line",
		id=item.id or widget.id,
		class_hooks=item.class_hooks or widget.class_hooks,
		hooks=item.hooks or widget.hooks,
		hx=ss,
		hy=ss,
		text_align="left",
		user=item,
		color=0,
		solid=true,
	}

	item.line=widget:create(opts)
	item.line_indent=item.line:add({class="text",text=(" "):rep(item.depth)})
	if item.mode=="directory" then
		if item.expanded then
			item.line_prefix=item.line:add({class="text",text="= "})
		else
			item.line_prefix=item.line:add({class="text",text="> "})
		end
		item.line_text=item.line:add({class="text",text=item.name.."/"})
	else
		item.line_prefix=item.line:add({class="text",text="- "})
		item.line_text=item.line:add({class="text",text=item.name})
	end

end

function wtreefile.item_toggle_dir(item)
	if item.expanded then
		wtreefile.item_empty_dir(item)
	else
		wtreefile.item_fill_dir(item)
	end
end

function wtreefile.item_empty_dir(item)
	item.expanded=false
	for i=#item,1,-1 do
		local it=item[i]
		if not it.keep then
			table.remove(item,i)	-- remove item
		end
	end
end

function wtreefile.item_fill_dir(item)

	wtreefile.item_empty_dir(item)

	item.expanded=true	

	local files={}
	pcall( function()
		for n in lfs.dir(item.path) do
			if n~="." and n~=".." then
				local t=lfs.attributes(wpath.resolve(item.path,n))
				if t then
					t.name=n
					t.path=wpath.resolve(item.path,t.name)
					files[#files+1]=t
				end
			end
		end
	end)
	
	for i,file in ipairs(files) do
	
		local it
		for _,v in ipairs(item) do
			if v.name==file.name then it=v break end -- already have this one
		end
		if not it then -- does not already exist
			local it={}
			it.mode=file.mode
			it.path=file.path
			it.name=file.name
			it.depth=item.depth+1
			it.refresh=wtreefile.refresh_item

			item[#item+1]=it
		end

	end

	table.sort(item,function(a,b)
		if a.mode == b.mode then
			return b.name:lower() > a.name:lower()
		else
			return b.mode > a.mode
		end
	end)


end

function wtreefile.refresh(treefile)

	local items={}
	local pp=wpath.split(treefile.data_dir:value())
	if pp[#pp]=="" then pp[#pp]=nil end -- strip trailing slash
--		dprint(pp)
	local path=""
	local last=items
	for i,v in ipairs(pp) do
		path=path..v.."/"
		local it={
			name=v,
			text=v.."/",
			path=path,
			mode="directory",
			depth=i-1,
			refresh=wtreefile.refresh_item,
			keep=true, -- no not remove when collapsed
		}
		if it.refresh then it:refresh(treefile.tree_widget) end
		last[#last+1]=it
		last=it
	end
	wtreefile.item_fill_dir(last)
	
	treefile.tree_widget:refresh_items(items)
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

				wtreefile.item_toggle_dir(it)

				if it.refresh then it:refresh(treefile.tree_widget) end

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

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
	

function wtreefile.refresh_items(items)

	local files={}
	pcall( function()
		for n in lfs.dir(items.path) do
			if n~="." and n~=".." then
				local t=lfs.attributes(wpath.resolve(items.path,n))
				if t then
					t.name=n
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
		it.path=wpath.resolve(items.path,v.name)

		if v.mode=="directory" then
			it.text=v.name.."/"
		else
			it.text=v.name
		end

		items[#items+1]=it
		
	end

end

function wtreefile.refresh(widget)


	local items={}

	items.path=wpath.resolve(".")
	items.mode="directory"
	wtreefile.refresh_items(items)

	widget.tree_widget:refresh(items)

end

function wtreefile.class_hooks(hook,widget,dat)
	if hook=="click" and widget and widget.id=="files" then
		local it=widget.user
		local tree=widget ; while tree and tree.parent~=tree and tree.class~="tree" do tree=tree.parent end
		local treefile=tree.parent
		if treefile.class=="treefile" then -- sanity

			if it.mode=="directory" then

				if it[1] then
					for i=#it,1,-1 do
						it[i]=nil
					end
				else
					wtreefile.refresh_items(it)
				end

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
	widget.class_hooks=wtreefile.class_hooks


	widget.tree_widget=widget:add({hx=widget.hx,hy=widget.hy,size="full",class="tree",id="files"})
	widget.tree_widget.hooks=wtreefile.class_hooks

	widget:refresh()

	return widget
end


	return wtreefile
end

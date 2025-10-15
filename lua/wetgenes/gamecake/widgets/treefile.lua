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


M.mount_fs=function()
	return M.fs.setup({
		name="/",
		path="/",
		dir={},
	})
end


M.fs={}
-- simple inherits, no tables and last has priority
for _,t in ipairs({
	{ "wetgenes.gamecake.widgets.tree" , "ms" },
}) do
	for n,v in pairs( require( t[1] )[ t[2] ] ) do
		if type(v)~="table" then
			M.fs[n]=v
		end
	end
end

M.fs_metatable={__index=M.fs}

local wpath=require("wetgenes.path")
local lfs ; pcall( function() lfs=require("lfs") end )

M.fs.is="fs"

M.fs.setup=function(fs)
	if not fs then fs={} end
	setmetatable(fs,M.fs_metatable)		
	if fs.path and not fs.name then -- fill in name etc from path
		local pp=wpath.parse( wpath.unslash( wpath.resolve(fs.path) ) )
		fs.name=pp.file
		pcall( function() -- lfs may not exist
			local attr=lfs.attributes(fs.path)
			if attr then
				local it={}
				if attr.mode=="directory" then
					fs.expanded=false -- dirs start of collapsed
					fs.dir={}
					fs.name=fs.name.."/"
				else
					fs.size=attr.size
				end
			end
		end)
	end
	return fs
end

M.fs.fetch_attr=function(fs,path)
	path=wpath.resolve(path)
	local attr
	pcall( function()
		attr=lfs.attributes(path)
	end )
	return attr
end

M.fs.fetch_dir=function(fs,path)
	path=wpath.resolve(path)
	local dir={}
	pcall( function()
		for n in lfs.dir(path) do
			if n~="." and n~=".." then
				local it=fs.setup({path=wpath.resolve(path,n)})
				it.parent=fs
				dir[#dir+1]=it
			end
		end
	end)
	return dir
end

M.fs.manifest_path=function(fs,fullpath)
	fullpath=wpath.unslash(fullpath)
	local pp=wpath.split(fullpath)
--	if pp[#pp]=="" then pp[#pp]=nil end -- strip trailing slash

	local path=""
	local last=fs
	for i,v in ipairs(pp) do
		path=wpath.resolve(path.."/"..v)
		local it=last:find_it({path=path})
		if not it then -- need to create
			it=fs.setup({
				path=path,
			})
			last.dir[#last.dir+1]=it
		end
		it.keep=true -- do not remove this one when collapsed
		last=it
	end

	return last
end



M.fs.hooks=function(hook,widget,dat)

-- print(hook,widget,dat,widget and widget.user and widget.user.path , widget.class )
	
	if hook=="click" and widget and widget.class=="line" then
		local it=widget.user
		local tree=widget ; while tree and tree.parent~=tree and tree.class~="tree" do tree=tree.parent end
		assert( tree.class=="tree" )
		
		if it.dir then
			it:toggle_dir()
			tree:refresh()
		end

		tree:call_hook_later("line_click",it)
	end
end



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
	widget.tree_widget.items=require("wetgenes.gamecake.widgets.treefile").mount_fs()
	widget.tree_widget.items:manifest_path( widget.data_dir:value() )
	widget.tree_widget.hooks=function(...) return widget.hooks(...) end -- hook up sub widget

	return widget
end


	return wtreefile
end

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

M.fs.sort_dir=function(fs)
	if fs.dir then -- safe check
		table.sort(fs.dir,function(a,b)
			if ( not a.dir ) == ( not b.dir ) then -- both are dir or files
				return a.name:lower() < b.name:lower()
			else
				if a.dir then return true end
			end
		end)
	end
end

M.fs.merge_dir=function(fs,dir)

	if not dir then
		dir=fs:fetch_dir(fs.path)
	end

	if not fs.dir then -- just set
		fs.dir=dir
	else

		local map={}
		for i,v in ipairs(dir) do
			map[v.name]=v
		end

		for i=#fs.dir,1,-1 do -- backwards so we can remove
			local v=fs.dir[i]
			local o=map[v.name]
			if o then -- merge
				map[v.name]=nil -- forget after merge 
				for k,s in pairs(o) do
					if not v[k] or type(s)~="table" then -- do not merge tables unless dest is empty
						v[k]=s
					end
				end
			else -- delete
				table.remove(fs.dir,i)
			end
		end
		-- any items left in the map are new
		for _,v in pairs(map) do -- add
			fs.dir[#fs.dir+1]=v
		end

	end

	fs:sort_dir()
end


M.fs.find_it=function(fs,search)
	local found=true
	for k,v in pairs(search) do -- must match every key,value in search
		if fs[k]~=v then found=false break end
	end
	if found then return fs end -- found it
	
	if fs.dir then -- is a dir with data
		for ids,it in pairs(fs.dir) do
			local found=it:find_it(search)
			if found then return found end -- found it
		end
	end
	
	return false
end


M.fs.manifest_path=function(fs,fullpath)
	fullpath=wpath.unslash(fullpath)
	local pp=wpath.split(fullpath)
	if pp[#pp]=="" then pp[#pp]=nil end -- strip trailing slash

	local path=""
	local last=fs
	for i,v in ipairs(pp) do
		path=path..v.."/"
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

end

M.fs.toggle_dir=function(fs)

	if not fs.dir then return end
	
	fs.expanded = not fs.expanded

	if fs.expanded then -- expanded so refresh contents
		fs:merge_dir()
	else -- collapsed so remove children
		for i=#fs.dir,1,-1 do
			local v=fs.dir[i]
			if not v.keep then
				table.remove(fs.dir,i)
			end
		end		
	end

end


M.fs.to_line=function(fs,widget,only)

	local ss=widget.master.theme.grid_size		
	local opts={
		class="line",
		class_hooks=widget.class_hooks,
		hooks=fs.hooks, -- or widget.hooks,
		hx=ss,
		hy=ss,
		text_align="left",
		user=fs,
		color=0,
		solid=true,
	}

	fs.line=widget:create(opts)

	local pp=wpath.split( wpath.unslash(fs.path) )

	fs.text=string.rep(" ",#pp-1)
	if fs.dir then
		if fs.expanded then
			fs.text=fs.text.."< "
		else
			fs.text=fs.text.."= "
		end
	else
		fs.text=fs.text.."- "
	end
	fs.text=fs.text..fs.name

	fs.line_text=fs.line:add({class="text",text=fs.text})

	if not only then
		if fs.dir then
			for _,it in ipairs(fs.dir) do
				it:to_line(widget)
			end
		end
	end
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

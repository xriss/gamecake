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


M.mount_meta=function(...)
	return M.meta.setup({
		name="",
		path="",
		dir={...},
	})
end


M.meta={}
M.meta_metatable={__index=M.meta}

local wpath=require("wetgenes.path")

M.meta.is="meta"

M.meta.find_prefix=function(meta,path)
	local best
	for _,it in pairs(meta.dir) do -- find best dir prefix for this path ( mount points )
		local c=path:sub(1,#it.path) -- path must begin with
		if c==it.path then
			if not best then best=it end
			if #it.path > #best.path then best=it end -- longest path wins
		end
	end
	return best,path -- may be nil and path may be adjusted
end

M.meta.setup=function(meta)
	if not meta then meta={} end
	setmetatable(meta,M.meta_metatable)		
	return meta
end

M.meta.sort_dir=function(meta)
	if meta.dir then -- safe check
		table.sort(meta.dir,function(a,b)
			if ( not a.dir ) == ( not b.dir ) then -- both are dir or files
				return a.name:lower() < b.name:lower()
			else
				if a.dir then return true end
			end
		end)
	end
end

M.meta.merge_dir=function(meta,dir)

	if not dir then
		dir=meta:fetch_dir(meta.path)
		if not dir then
			return -- nothing to do
		end
	end

	if not meta.dir then -- just set
		meta.dir=dir
	else

		local map={}
		for i,v in ipairs(dir) do
			map[v.name]=v
		end

		for i=#meta.dir,1,-1 do -- backwards so we can remove
			local v=meta.dir[i]
			local o=map[v.name]
			if o then -- merge
				map[v.name]=nil -- forget after merge 
				for k,s in pairs(o) do
					if not v[k] or type(s)~="table" then -- do not merge tables unless dest is empty
						v[k]=s
					end
				end
			else -- delete
				table.remove(meta.dir,i)
			end
		end
		-- any items left in the map are new
		for _,v in pairs(map) do -- add
			meta.dir[#meta.dir+1]=v
		end

	end

	meta:sort_dir()
end

M.meta.toggle_dir=function(meta)

	if not meta.dir then return end
	
	meta.expanded = not meta.expanded

	if meta.expanded then -- expanded so refresh contents
		meta:merge_dir()
	else -- collapsed so remove children
		for i=#meta.dir,1,-1 do
			local v=meta.dir[i]
			if not v.keep then
				table.remove(meta.dir,i)
			end
		end		
	end

end

M.meta.find_it=function(meta,search)
	local found=true
	for k,v in pairs(search) do -- must match every key,value in search
		if meta[k]~=v then found=false break end
	end
	if found then return meta end -- found it
	
	if meta.dir then -- is a dir with data
		for ids,it in pairs(meta.dir) do
			local found=it:find_it(search)
			if found then return found end -- found it
		end
	end
	
	return false
end


M.meta.to_line=function(meta,widget,only)

	if meta.path~="" then -- no show base mounts
	
		local ss=widget.master.theme.grid_size		
		local opts={
			class="line",
			class_hooks=widget.class_hooks,
			hooks=meta.hooks, -- or widget.hooks,
			hx=ss,
			hy=ss,
			text_align="left",
			user=meta,
			color=0,
			solid=true,
		}

		meta.line=widget:create(opts)

		local pp=wpath.split( wpath.unslash(meta.path) )

		meta.text=string.rep(" ",#pp-1)
		if meta.dir then
			if meta.expanded then
				meta.text=meta.text.."< "
			else
				meta.text=meta.text.."= "
			end
		else
			meta.text=meta.text.."- "
		end
		meta.text=meta.text..meta.name

		meta.line_text=meta.line:add({class="text",text=meta.text})

	end

	if not only then
		if meta.dir then
			for _,it in ipairs(meta.dir) do
				it:to_line(widget)
			end
		end
	end
end


M.meta.fetch_attr=function(meta,path)
	local it,path=meta:find_prefix(path)
	if it and it.fetch_attr then
		return it:fetch_attr(path)
	end
end

M.meta.fetch_dir=function(meta,path)
	local it,path=meta:find_prefix(path)
	if it and it.fetch_dir then
		return it:fetch_dir(path)
	end
end

M.meta.manifest_path=function(meta,path)
	local it,path=meta:find_prefix(path)
	if it and it.manifest_path then
		return it:manifest_path(path)
	end
end

M.meta.read_file=function(meta,path)
	local it,path=meta:find_prefix(path)
	if it and it.read_file then
		return it:read_file(path)
	end
end

M.meta.write_file=function(meta,path,data)
	local it,path=meta:find_prefix(path)
	if it and it.write_file then
		return it:write_file(path,data)
	end
end

M.meta.hooks=function(hook,widget,dat)

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



M.mount_file=function()
	return M.file.setup({
		name="/",
		path="/",
		dir={},
	})
end


M.file={}
-- simple inherits, no tables and last has priority
do
	for _,it in ipairs{ M.meta , } do
		for n,v in pairs( it ) do
			if type(v)~="table" then
				M.file[n]=v
			end
		end
	end
end

M.file_metatable={__index=M.file}

local wpath=require("wetgenes.path")
local lfs ; pcall( function() lfs=require("lfs") end )

M.file.is="file"

M.file.setup=function(file)
	if not file then file={} end
	setmetatable(file,M.file_metatable)		
	if file.path and not file.name then -- fill in name etc from path
		local pp=wpath.parse( wpath.unslash( wpath.resolve(file.path) ) )
		file.name=pp.file
		pcall( function() -- lfs may not exist
			local attr=lfs.attributes(file.path)
			if attr then
				local it={}
				if attr.mode=="directory" then
					file.expanded=false -- dirs start of collapsed
					file.dir={}
					file.name=file.name.."/"
				else
					file.size=attr.size
				end
			end
		end)
	end
	return file
end

M.file.fetch_attr=function(file,path)
	path=wpath.resolve(path)
	local attr
	pcall( function()
		attr=lfs.attributes(path)
	end )
	return attr
end

M.file.fetch_dir=function(file,path)
	path=wpath.resolve(path)
	local dir={}
	pcall( function()
		for n in lfs.dir(path) do
			if n~="." and n~=".." then
				local it=file.setup({path=wpath.resolve(path,n)})
				it.parent=file
				dir[#dir+1]=it
			end
		end
	end)
	return dir
end

M.file.read_file=function(_,path)
	local text
	local f=io.open(path,"rb") -- read full file
	if f then
		text=f:read("*a")
		f:close()
	end
	return text
end

M.file.write_file=function(_,path,text)
	local f=io.open(it.meta.path,"wb")
	if f then
		local d=f:write(text)
		f:close()
		return true
	end
	return false
end


M.file.manifest_path=function(file,fullpath)
	fullpath=wpath.unslash(fullpath)
	local pp=wpath.split(fullpath)
--	if pp[#pp]=="" then pp[#pp]=nil end -- strip trailing slash

	local path=""
	local last=file
	for i,v in ipairs(pp) do
		path=wpath.resolve(path.."/"..v)
		local it=last:find_it({path=path})
		if not it then -- need to create
			it=file.setup({
				path=path,
			})
			last.dir[#last.dir+1]=it
		end
		it.keep=true -- do not remove this one when collapsed
		last=it
	end

	return last
end



M.mount_gist=function()
	return M.gist.setup({
		name="//gists/",
		path="//gists/",
		dir={},
	})
end

M.gist={}
-- simple inherits, no tables and last has priority
do
	for _,it in ipairs{ M.meta , } do
		for n,v in pairs( it ) do
			if type(v)~="table" then
				M.gist[n]=v
			end
		end
	end
end

M.gist_metatable={__index=M.gist}

local wpath=require("wetgenes.path")

M.gist.is="gist"

M.gist.setup=function(gist)
	if not gist then gist={} end
	setmetatable(gist,M.gist_metatable)		
	if gist.path and not gist.name then -- fill in name etc from path
		local pp=wpath.parse( wpath.unslash( wpath.resolve(gist.path) ) )
		gist.name=pp.file
	end
	return gist
end


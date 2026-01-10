--
-- (C) 2025 Kriss@XIXs.com
--


local djon=require("djon")
local wpath=require("wetgenes.path")
local lfs ; pcall( function() lfs=require("lfs") end )
local wgist=require("wetgenes.tasks_gist")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


--------------------------------------------------------------------------------


M.meta={}
M.meta_metatable={__index=M.meta}

M.meta.is="meta"

M.mount_meta=function(...)
	return M.meta.setup({
		name="",
		path="",
		dir={...},
		keep=true,
	})
end

M.meta.setup=function(meta)
	if not meta then meta={} end
	setmetatable(meta,M.meta_metatable)
	if meta.mounts then
		meta:merge_dir(meta.mounts)
	end
	return meta
end


M.meta.find_path=function(meta,path)
	local ret
	local walk;walk=function(it)
		if it.path==path then -- found it
print("found",it.path)
			ret=it
		end
print("walk",it.path)
		if it.path==path:sub(1,#it.path) then -- only look here
			if it.dir then
				for i,v in ipairs(it.dir or {}) do
					walk(v)
				end
			end
		end
	end
	walk(meta)
	return ret
end

M.meta.find_prefix=function(meta,path)
	local best=meta
	for _,it in pairs(meta.mounts or {}) do -- find best dir prefix for this path ( mount points )
		local c=path:sub(1,#it.path) -- path must begin with
		if c==it.path then
			if not best then best=it end
			if #it.path > #best.path then best=it end -- longest path wins
		end
	end
	if best~=meta and best.mounts then -- auto recursive on sub mounts find
		local it,path=best:find_prefix(path)
		if not best then best=it end
		if #it.path > #best.path then best=it end -- longest path wins
	end
	return best,path -- may be nil and path may be adjusted
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

M.meta.merge_dir=function(meta,dir,only)

	if not dir then
		dir=meta:fetch_dir(meta.path)
		if meta.mounts then
			for _,m in ipairs(meta.mounts) do
				dir=dir or {}
				dir[#dir+1]=m
			end
		end
		if not dir then
			return -- nothing to do
		end
	end
	
	if not meta.dir then meta.dir={} end

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
			if only then -- delete everything else
				if not v.keep then -- unless we want to keep it
					table.remove(meta.dir,i)
				end
			end
		end
	end
	-- any items left in the map are new
	for _,v in pairs(map) do -- add
		meta.dir[#meta.dir+1]=v
	end

	meta:sort_dir()
end

M.meta.toggle_dir=function(meta)

	if not meta.dir then return end
	
	meta.expanded = not meta.expanded

	if meta.expanded then -- expanded so refresh contents
		meta:merge_dir(nil,true)
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

		if meta.indent then  -- custom indent
			meta.text=meta.indent
		else -- count slashes
			local pp=#(wpath.split( wpath.unslash(meta.path) ))-1
	--		if meta.path:sub(1,2)=="/../" then pp=pp-2 else pp=pp-1 end
			meta.text=string.rep(" ",pp)
		end

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
	
	if type(meta.keep)=="table" then -- a bool flag that can be a table
		if meta.keep.to_line then -- callback to modify meta.line_text etc
			meta.keep.to_line( meta , widget )
		end
	end
end


M.meta.fetch_dir=function(meta,path)
	local it,path=meta:find_prefix(path)
	if it and it.fetch_dir and it~=meta then
		return it:fetch_dir(path)
	end
end

M.meta.new_item=function(meta,name)
	local path=meta.path..name
	if meta.mounts then -- use auto mount items ?
		for n,v in pairs(meta.mounts) do
			if v.path==path then return v end
		end
	end
	local isdir
	if name:sub(-1)=="/" then isdir={} end -- use trailing slash as dir flag
	local it=meta.setup({
		path=path,
		parent=meta,
		dir=isdir,
	})
	return it
end

M.meta.manifest_path=function(meta,fullpath)

	local best,fullpath=meta:find_prefix(fullpath)
	local path=best.path
	fullpath=fullpath:sub(#path+1)
--	best.keep=true

	fullpath=wpath.unslash(fullpath)
	local pp=wpath.split(fullpath)
--	if pp[#pp]=="" then pp[#pp]=nil end -- strip trailing slash
	local last=best
	for i,v in ipairs(pp) do
		if pp[i+1] then v=v.."/" end -- trailing slash except for last
		path=path..v
		local it=last:find_it({path=path})
		if not it then -- need to create
			it=last:new_item(v)
			if last.dir then
				last:merge_dir({it})
--				last.dir[#last.dir+1]=it
			end
		end
--		it.keep=true -- do not remove this one when collapsed
		last=it
	end

	return last

end

M.meta.read_file=function(meta,path)
print("READ",meta.is,path)
	local it,path=meta:find_prefix(path)
	if it and it.read_file then
print("READ",it.path,it.is,path)
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

PRINT("click",it.path)

		if it.dir then
			it:toggle_dir()
			tree:refresh()
		end

		tree:call_hook_later("line_click",it)
	end
end


--------------------------------------------------------------------------------


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

M.file.is="file"

M.mount_file=function()
	return M.file.setup({
		name="/",
		path="/",
		dir={},
		keep=true,
	})
end

M.file.setup=function(file)
	if not file then file={} end
	setmetatable(file,M.file_metatable)		
	if file.mounts then
		file:merge_dir(file.mounts)
	end
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
					if file.path:sub(-1)~="/" then -- force ending to slash if dir
						file.path=file.path.."/"
					end
					file.name=file.name.."/"
				else
					file.size=attr.size
				end
			end
		end)
	end
	return file
end

M.file.new_item=function(file,name)
	local it=file.setup({path=file.path..name,parent=file})
	return it
end

M.file.fetch_dir=function(file,path)
	path=wpath.resolve(path)
	local dir={}
	pcall( function()
		for n in lfs.dir(path) do
			if n~="." and n~=".." then
				dir[#dir+1]=file:new_item(n)
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
	local f=io.open(path,"wb")
	if f then
		local d=f:write(text)
		f:close()
		return text -- success
	end
end


--------------------------------------------------------------------------------


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

M.gist.is="gist"

M.mount_gist=function()
	return M.gist.setup({
		name="/../gists/",
		path="/../gists/",
		dir={},
		keep=true,
	})
end

M.gist.setup=function(gist)
	if not gist then gist={} end
	setmetatable(gist,M.gist_metatable)		
	if gist.mounts then
		gist:merge_dir(gist.mounts)
	end
	if gist.path and not gist.name then -- fill in name etc from path
		local pp=wpath.parse( wpath.unslash( wpath.resolve(gist.path) ) ) -- note this will lose one of the two stating slashes
		gist.name=pp.file
		if gist.dir then gist.name=gist.name.."/" end
	end
	return gist
end

M.gist.new_item=function(gist,name)
	local isdir
	if name:sub(-1)=="/" then isdir={} end -- use trailing slash as dir flag
	local it=gist.setup({
		path=gist.path..name,
		collect=gist.collect,
		dir=isdir,
		parent=gist,
	})
	return it
end

M.gist.fetch_dir=function(gist,path)
--	path="/"..wpath.resolve(path) -- force the // prefix
	local dir={}

	local pp=wpath.split(path)

	local gid,gfname
	if pp[1]=="" and pp[2]==".." and pp[3]=="gists" then
		gid=pp[4] or ""
		gid=gid:match("[^%.]*$") -- id is end part, possibly after last .
		gfname=pp[5]
	end
	if gid=="" then gid=nil end
	if gfname=="" then gfname=nil end

	if path=="/../gists/" then
		local tab=gist.collect.config.cloud and gist.collect.config.cloud.gist_bookmarks or {}
		for i,v in ipairs(tab) do
			dir[#dir+1]=gist:new_item(v.."/")
		end

	elseif gid and not gfname then -- hit up gist

		local opts={}
		opts.tasks=gist.collect.oven.tasks
		opts.gid=gid

		local result=wgist.get(opts)
		for n,v in pairs(result.files or {}) do
			dir[#dir+1]=gist:new_item(n)
		end
		dir[#dir+1]=gist:new_item(".meta")

	end
	
	return dir
end

M.gist.read_file=function(gist,path)
PRINT("read_gist",path)

	local pp=wpath.split(path)
	local gid,gfname
	if pp[1]=="" and pp[2]==".." and pp[3]=="gists" then
		gid=pp[4]
		gid=gid:match("[^%.]*$") -- id is end part, possibly after last .
		gfname=pp[5]
	end
	if gid=="" then gid=nil end
	if gfname=="" then gfname=nil end
	
	if gid and gfname then
	
		local opts={}
		opts.tasks=gist.collect.oven.tasks
		opts.gid=gid

		local result=wgist.get(opts)
		if gfname==".meta" then
			return djon.save({
				description=result.description,
				owner=result.owner and result.owner.login,
				created_at=result.created_at,
				updated_at=result.updated_at,
				public=result.public,
				html_url=result.html_url,
			},"djon")
		else
			return result.files and result.files[gfname] and result.files[gfname].content
		end
	end

end

M.gist.write_file=function(gist,path,data)
PRINT("write_gist",path)

	local pp=wpath.split(path)
	local gid,gfname
	if pp[1]=="" and pp[2]==".." and pp[3]=="gists" then
		gid=pp[4]
		gid=gid:match("[^%.]*$") -- id is end part, possibly after last .
		gfname=pp[5]
	end
	if gid=="" then gid=nil end
	if gfname=="" then gfname=nil end

	if gid and gfname then

		if not gist.collect.config.cloud.gist_token then
			return nil
		end
	
		local opts={}
		opts.tasks=gist.collect.oven.tasks
		opts.gid=gid
		opts.token=gist.collect.config.cloud.gist_token
		
		if gfname==".meta" then -- no save meta

			return data
		else
			opts.body={files={}}
			opts.body.files[gfname]={content=data}
			local result=wgist.set(opts)
-- we get result?
			return result.files and result.files[gfname] and result.files[gfname].content
		end

	end

end


--------------------------------------------------------------------------------


M.config={}
-- simple inherits, no tables and last has priority
do
	for _,it in ipairs{ M.meta , } do
		for n,v in pairs( it ) do
			if type(v)~="table" then
				M.config[n]=v
			end
		end
	end
end

M.config_metatable={__index=M.config}

local wpath=require("wetgenes.path")

M.config.is="config"

M.mount_config=function()
	return M.config.setup({
		name="/../config/",
		path="/../config/",
		dir={},
		keep=true,
	})
end

M.config.setup=function(config)
	if not config then config={} end
	setmetatable(config,M.config_metatable)		
	if config.mounts then
		config:merge_dir(config.mounts)
	end
	if config.path and not config.name then -- fill in name etc from path
		local pp=wpath.parse( wpath.unslash( wpath.resolve(config.path) ) ) -- note this will lose one of the two stating slashes
		config.name=pp.file
	end
	return config
end

M.config.new_item=function(config,name)
	local it=config.setup({
		path=config.path..name,
		collect=config.collect,
		parent=config,
	})
	return it
end

M.config.fetch_dir=function(config,path)
--	path="/"..wpath.resolve(path) -- force the // prefix
	local dir={}

	if path=="/../config/" then

		local rows=config.collect.do_memo({
			sql=[[

	SELECT key FROM config ;

			]],
		}).rows
		for i,v in ipairs(rows) do
			dir[#dir+1]=config:new_item(v.key..".djon")
		end

	end
	
	return dir
end

M.config.read_file=function(config,path)
PRINT("read_config",path)
	if path:sub(1,#"/../config/")=="/../config/" then
		local key=path:sub(1+#"/../config/")
		if key:sub(-#".djon")==".djon" then key=key:sub(1,-(1+#".djon")) end -- remove extension
		local rows=config.collect.do_memo({
			binds={
				KEY=key,
			},
			sql=[[

	SELECT key,value FROM config WHERE key=$KEY;

			]],
		}).rows
		return rows and rows[1] and rows[1].value
	end
end

M.config.write_file=function(config,path,data)
PRINT("write_config",path)

	if path:sub(1,#"/../config/")=="/../config/" then

		local key=path:sub(1+#"/../config/")
		if key:sub(-#".djon")==".djon" then key=key:sub(1,-(1+#".djon")) end -- remove extension
		local tab
		pcall(function() -- try and parse but ignore errors
			tab=djon.load(data,"comments")
			config.collect.config[key]=djon.load(data) -- set internal config
		end)
		tab=tab or {{}} -- maybe wipe on error , undo to get old text back
		local newdata=djon.save(tab,"djon","comments")  -- reformat keeping comments

		local rows=config.collect.do_memo({
			binds={
				KEY=key,
				VALUE=newdata,
			},
			sql=[[

	INSERT INTO config(key,value) VALUES( $KEY , $VALUE ) ON CONFLICT(key) DO UPDATE SET value=$VALUE ;

			]],
		})

		return newdata
	end
end


--------------------------------------------------------------------------------


M.readme={}
-- simple inherits, no tables and last has priority
do
	for _,it in ipairs{ M.meta , } do
		for n,v in pairs( it ) do
			if type(v)~="table" then
				M.readme[n]=v
			end
		end
	end
end

M.readme_metatable={__index=M.readme}

local wpath=require("wetgenes.path")

M.readme.is="readme"

M.mount_readme=function()
	return M.readme.setup({
		name="/../readme/",
		path="/../readme/",
		dir={},
		keep=true,
	})
end

M.readme.setup=function(readme)
	if not readme then readme={} end
	setmetatable(readme,M.readme_metatable)		
	if readme.mounts then
		readme:merge_dir(readme.mounts)
	end
	if readme.path and not readme.name then -- fill in name etc from path
		local pp=wpath.parse( wpath.unslash( wpath.resolve(readme.path) ) )
		readme.name=pp.file
	end
	return readme
end

M.readme.new_item=function(readme,name)
	local it=readme.setup({
		path=readme.path..name,
		collect=readme.collect,
		parent=readme,
	})
	return it
end

M.readme.fetch_dir=function(readme,path)
--	path="/"..wpath.resolve(path) -- force the // prefix
	local dir={}

	if path=="/../readme/" then
	
		local rows={}
		for n,v in pairs( readme.collect.readmes ) do
			dir[#dir+1]=readme:new_item(n)
		end

	end
	
	return dir
end

M.readme.read_file=function(readme,path)
PRINT("read_readme",path)
	if path:sub(1,#"/../readme/")=="/../readme/" then
		local key=path:sub(1+#"/../readme/")		
		return readme.collect.readmes[key]
	end
end

M.readme.write_file=function(readme,path,data)
PRINT("write_readme",path)

	if path:sub(1,#"/../readme/")=="/../readme/" then

		local key=path:sub(1+#"/../readme/")
		return nil -- no can save
	end
end


--------------------------------------------------------------------------------


M.find={}
-- simple inherits, no tables and last has priority
do
	for _,it in ipairs{ M.meta , } do
		for n,v in pairs( it ) do
			if type(v)~="table" then
				M.find[n]=v
			end
		end
	end
end

M.find_metatable={__index=M.find}

local wpath=require("wetgenes.path")

M.find.is="find"

M.mount_find=function()
	return M.find.setup({
		name="/../find/",
		path="/../find/",
		dir={},
		keep=true,
	})
end

M.find.setup=function(find)
	if not find then find={} end
	setmetatable(find,M.find_metatable)		
	if find.mounts then
		find:merge_dir(find.mounts)
	end
	if find.path and not find.name then -- fill in name etc from path
		local pp=wpath.parse( wpath.unslash( wpath.resolve(find.path) ) )
		find.name=pp.file
	end
	return find
end

M.find.new_item=function(find,name)
	local it=find.setup({
		path=name,
		indent="   ",
		name=name,
		collect=find.collect,
		find=find.find,
		parent=find,
	})
	return it
end

M.find.fetch_dir=function(find,path)
--	path="/"..wpath.resolve(path) -- force the // prefix
	local dir={}
	
	local walkcount=function(tree)
		local count=0
		local walk;walk=function(t)
			for i,v in pairs(t) do
				if type(v)=="table" then
					walk(v)
				else
					count=count+1
				end
			end
		end
		walk(tree)
		return count
	end
	
	if path=="/../find/" then

PRINT("list_find",path)

		local rows={}

		local it=find.collect.finds.list[1] -- only 1 find active
		if it then
			local n=next(it.filetree) -- only 1 root at top level
			if n then
				local v=it.filetree[n]
				local count=walkcount(v)
				local f=find:new_item(n)
				dir[#dir+1]=f
				f.name="("..count..")"..n
				f.filetree=v
				f.dir={}
				f.indent="    "
				f.find=it
			end
		end

	else

		for n,v in pairs( find.filetree ) do
		
			if type(v)=="table" then
				local count=walkcount(v)
				local f=find:new_item(find.path..n)
				dir[#dir+1]=f
				f.name="("..count..")"..n
				f.filetree=v
				f.dir={}
				f.indent=find.indent.." "
			else
				local f=find:new_item(find.path..n)
				f.name="<"..v..">"..n
				dir[#dir+1]=f
				f.indent=find.indent.." "
			end
		
		end

	end
	
	return dir
end

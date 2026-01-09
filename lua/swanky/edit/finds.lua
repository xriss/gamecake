--
-- (C) 2025 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local wpath=require("wetgenes.path")

local lfs ; pcall( function() lfs=require("lfs") end )

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local wildcard_pattern=function(s)
	return s:gsub("[%(%)%.%%%+%–%*%?%[%^%$]",
		function(a)
			if a=="*" then
				return ".*"
			elseif a=="?" then
				return "."
			else
				return "%"..a
			end
		end
	)
end

local map_to_tree=function(map)

	local tree={}
	
	local root
	for n,v in pairs(map) do
		if not root then
			root=n
		else
			for i=1,#root do
				if root:sub(i,i)~=n:sub(i,i) then
					root=root:sub(1,i-1)
					break
				end
			end
		end
	end
	
	if not root then return tree end -- no data
	
	while #root>0 do
		if root:sub(-1,-1)=="/" then
			break
		else
			root=root:sub(1,-2) -- must end on /
		end
	end
	
	tree[root]={}
	for n,v in pairs(map) do
		local cn=n:sub(#root+1)
		local fn=cn:find("/[^/]*$")
		local d,f
		if fn then
			d=cn:sub(1,fn)
			f=cn:sub(fn+1)
			if type(tree[root][d]) ~= "table" then
				tree[root][d]={}
			end
			tree[root][d][f]=v
		else
			tree[root][cn]=v
		end
	end

	return tree
end


-- searches

M.bake=function(oven,finds)

	local finds=finds or {}

	local collect=oven.rebake(oven.modname..".collect")
	local docs=oven.rebake(oven.modname..".docs")
	local gui=oven.rebake(oven.modname..".gui")
	local wtreefile=oven.rebake("wetgenes.gamecake.widgets.treefile")


	local find={}
	local find_meta={__index=find}

	finds.oven=oven	
	finds.modname=M.modname

	finds.list={} -- list of finds
	finds.tasks={}
	
	-- simple auto timeslice, writes "yield_maybe_time" in given table
	local yield_maybe=function(it)
		local now=wwin.time()					-- now time
		if	not it.yield_maybe_time or			-- starts off not set
			now>=(it.yield_maybe_time+0.10) or	-- yield after 10ms
			now<(it.yield_maybe_time)			-- time shenanigans
		then
			coroutine.yield()
			it.yield_maybe_time=wwin.time()		-- hopefully has ms resolution but maybe not
		end
	end

-- run the background find tasks
	finds.update=function()
		for find,task in pairs(finds.tasks) do
			if coroutine.status(task)=="dead" then
				finds.tasks[find]=nil -- forget
			else
				local suc,err=coroutine.resume(task) -- continue
				if not suc then
					error(err.."\n"..debug.traceback(task))
				end
			end
		end
	end

	finds.set_progress=function(idx,total)
		if idx and total then
			gui.datas.set_string("find_infiles",""..idx.."/"..total.."")
		else
			gui.datas.set_string("find_infiles","in files")
		end
	end

	finds.cancel_all=function()
print("cancel all")
		for i=#finds.list,1,-1 do
			local task=finds.list[i]
			finds.tasks[task]=nil
			finds.list[i]=nil
		end
		finds.set_progress()
		-- remove all expanded finds
		local mf=collect.mounts:find_path("/../find/")
		mf.dir={} -- empty
		if mf.expanded then
			mf:toggle_dir() -- force it closed
		end
		
		gui.refresh_tree()
	end

-- create the find
	finds.create=function(find)
		local find=find or {}
		setmetatable(find,find_meta)
		finds.list[#finds.list+1]=find
		find.finds=finds
		
		find.word=find.word or ""
		find.match=find.match or ""
		find.dir=find.dir or ""
		find.pattern=find.pattern or ".*"
		
		if find.match then -- get a dir from the match and convert wildcards to lua pattern
		
			local m=find.match
			local s=m:find("[^/]*[%?%*]") -- find first wildcard dir
			if not s then m=m.."*" -- auto *
				s=m:find("[^/]*[%?%*]") -- find first wildcard dir again
			end
		
			find.dir=wpath.resolve(m:sub(1,s-1)) -- directory to search
			find.pattern=wildcard_pattern( wpath.resolve( m ) ) -- full path of files to match
		end
print("MATCH",find.dir,find.pattern)

		return find
	end

-- find the find
	finds.get=function(dir,word)
		dir=wpath.unslash(dir)
		for i,find in ipairs( finds.list ) do
			if find.dir==dir and find.word==word then
				return find
			end
		end
	end

-- start scanning drive in a long running task
	find.scan=function(find)
	
		if not finds.tasks[find] then
		
			finds.tasks[find]=coroutine.create(function()
				return find:scan_task()
			end)
		
		end

	end
	find.scan_task=function(find)
		
--		local base_item=find:get_item()
--		if not base_item then return end -- must exist

		local dirs={}
		local files={}

		local filescan
		filescan=function(dir,match)
			yield_maybe(find)
			dirs[dir]=true
			pcall( function() -- maybe we do not have a filesystem
				for n in lfs.dir(dir) do
					if n~="." and n~=".." then
						local path=wpath.resolve(dir,n)
						local t=lfs.attributes(path)
						if t then
							if t.mode=="directory" then
								if not dirs[path] then -- avoid circular ?
									filescan(path)
								end
							elseif t.mode=="file" then							
								local s,e=path:find(find.pattern)
								if s==1 and e==#path then -- pattern must match full string
									files[path]=true
								end
							end
						end
					end
				end
			end)
		end
		filescan(wpath.resolve(find.dir)) -- find all the files

		find.filenames={}

		local count=0
		for file,_ in pairs(files) do count=count+1 end
--print( count )
		local idx=0
		for file,_ in pairs(files) do
			yield_maybe(find)
			idx=idx+1
--if idx%100==0 then print(idx) end

			finds.set_progress(idx,count)

			local fp
			local ok,err=pcall(function()
				fp=assert( io.open(file,"rb") )
				local d=fp:read("*all")
				if string.find(d,find.word,1,true) then -- found it
					find.filenames[file]=1
				end
			end)
			if not ok then print(file,err) end
			
			if fp then
				fp:close()
			end

			collectgarbage("step")
		end
		
		find.filetree=map_to_tree(find.filenames)
--		DUMP( find.filetree )
		
		finds.set_progress()
		local cnt=0
		for p,c in pairs( find.filenames ) do
			cnt=cnt+1
		end
		print("found "..cnt.."/"..count)

-- update tree
		local mf=collect.mounts:find_path("/../find/")
		if not mf.expanded then -- should be close
			mf:toggle_dir() -- force it open
		end
		for i,v in ipairs(mf.dir) do -- top level only
			if not v.expanded then -- should be close
				v:toggle_dir() -- force it open
			end
		end
		gui.refresh_tree()

	end

	return finds
end

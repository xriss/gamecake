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

-- searches

M.bake=function(oven,finds)

	local finds=finds or {}

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

	finds.cancel_all=function()
		for i=#finds.list,1,-1 do
			local task=finds.list[i]
			finds.tasks[task]=nil
			finds.list[i]=nil
		end
		gui.datas.set_string("find_infiles","in files")
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
		
			local s=find.match:find("[^/]*[%?%*]") -- find first wildcard dir
			if not s then s=#find.match+1 end -- full
		
			find.dir=wpath.resolve(find.match:sub(1,s-1)) -- directory to search
			find.pattern=wildcard_pattern( wpath.resolve( find.match ) ) -- full path of files to match
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
print( count )
		local idx=0
		for file,_ in pairs(files) do
			yield_maybe(find)
			idx=idx+1
if idx%100==0 then print(idx) end

			gui.datas.set_string("find_infiles",""..idx.."/"..count.."")

--print(file,"check")


--			base_item.text=idx.."/"..count
--			base_item.line_text.text=base_item.text
		
--print(file,idx,count)
			local fp=io.open(file,"rb")

--			pcall(function()
				if fp then
					local d=fp:read("*all")
					if string.find(d,find.word,1,true) then -- found it
--print(file,"found")
						find.filenames[file]=1
					else
--print(file)
					end
				else
print(file,"invalided")
				end
--			end)
			
			if fp then
				fp:close()
			end

		end
		
		finds.cancel_all()
		for p,c in pairs( find.filenames ) do
			print( p,c)
		end
	end

	return finds
end

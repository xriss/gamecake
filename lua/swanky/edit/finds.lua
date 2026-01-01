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
			now>=(it.yield_maybe_time+0.010) or	-- yield after 10ms
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
	end

-- create the find
	finds.create=function(find)
		local find=find or {}
		setmetatable(find,find_meta)
		finds.list[#finds.list+1]=find
		find.finds=finds
		
		find.dir=find.dir or ""
		find.word=find.word or ""
		
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
	
--[[
	finds.class_hook=function(hook,widget,dat)
		if hook=="click" then
			local it=widget.user
			local tree=widget ; while tree and tree.parent~=tree and tree.class~="tree" do tree=tree.parent end
			local treefile=tree.parent
			if it then -- item click
				treefile:call_hook_later("line_click",it)
--				print(a,b.user.mode)
			end
		end
--		print("hook",a,b,c,b and b.dir)
	end
	finds.class_hooks={finds.class_hook}

	find.item_refresh=function(find,item)
		local treefile=gui.master.ids.treefile

		local ss=treefile.master.theme.grid_size
		local opts={
			class="line",
			id=item.id or "find",
			class_hooks=finds.class_hooks,
			hooks=item.hooks, -- or treefile.hooks,
			hx=ss,
			hy=ss,
			text_align="left",
			user=item,
			color=0,
			solid=true,
		}

		if not item.line then
			item.line=treefile:create(opts)
			item.line_indent=item.line:add({class="text",text=(" "):rep(item.depth)})
			item.line_prefix=item.line:add({class="text",text="  "})
			item.line_text=item.line:add({class="text",text=item.text or item.word})
		end
		item.line_indent.text=(" "):rep(item.depth)
		if item.mode=="base_find" then
			item.line_prefix.text="> "
		elseif item.mode=="directory_find" then
			item.line_prefix.text="> "
		elseif item.mode=="file_find" then
			item.line_prefix.text="# "
		end
		item.line_text.text=item.text

		if item.mode=="file_find" or item.mode=="directory_find" then
			local loaded=docs.find(item.path)
			item.line_prefix.text_color=nil
			if loaded then
				item.line_prefix.text_color=0xff00cc00
				if loaded.meta.undo~=loaded.txt.undo.index then
					item.line_prefix.text_color=0xffcc0000
--					item.line_prefix.text="* "
--				else
--					item.line_prefix.text="+ "
				end
			end
		end
		
	end
	

-- add a tree item for this search
	find.add_item=function(find)
		local treefile=gui.master.ids.treefile

print("dir",find.dir)
		local dir_item=treefile:add_dir_item(find.dir)
print("path",dir_item.path)
		
		local item={
			mode="base_find",
			keep=true,
			path=dir_item.path,
			name="#"..find.word,
			dir=find.dir,
			word=find.word,
			text="searching",
			parent=dir_item,
			depth=dir_item.depth+1,
			refresh=function(item) return find:item_refresh(item) end,
		}
		dir_item[#dir_item+1]=item
		treefile:item_sort(dir_item)

		return item
	end

-- get the tree item for this search if it exists
	find.get_item=function(find)
		local treefile=gui.master.ids.treefile

		local rekky
		rekky=function(items)
			for _,item in ipairs(items) do
				if	item.mode == "base_find"    and
					find.dir == item.dir and 
					find.word == item.word
				then -- found it
					return item
				end
			end
			for _,item in ipairs(items) do
				if item[1] then -- recursive
					local ret=rekky(item)
					if ret then return ret end -- bubble up
				end
			end
		end
		local item=rekky(treefiles.items)

		return item
	end


	find.manifest_dir_item=function(find,base_item,dir)
		if dir=="" then return end
		
		local rpath=wpath.relative(base_item.dir,dir)	-- must be relative
		rpath=wpath.unslash(rpath)

		local pp=wpath.split(rpath)
		if pp[#pp]=="" then pp[#pp]=nil end -- strip trailing slash
		if pp[1]=="." then table.remove(pp,1) end -- strip starting dot

		local path=""
		local last=base_item
		for i,v in ipairs(pp) do
			local it
			path=path..v.."/"
			for ii,vv in ipairs(last) do -- find one that already exists
				if vv.mode=="directory_find" and vv.path==path then
					it=vv
					break
				end
			end
			if not it then -- need to create
				it={
					parent=last,
					text=v,
					name=v,
					path=path,
					mode="directory_find",
					depth=(last.depth or 0)+1,
					refresh=function(item) return find:item_refresh(item) end,
				}
				last[#last+1]=it
				it:refresh()
			end
			it.keep=true -- no not remove this one when collapsed
			last=it
		end

		return last
	end
]]


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
		filescan=function(dir)
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
								files[path]=true
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

		local idx=0
		for file,_ in pairs(files) do
			yield_maybe(find)
			idx=idx+1
--			base_item.text=idx.."/"..count
--			base_item.line_text.text=base_item.text
		
print(file,idx,count)
			local fp=io.open(file,"rb")

			pcall(function()
				local d=fp:read("*all")
				if string.find(d,find.word,1,true) then -- found it
print(file,"found")
					find.filenames[file]=1
				end
			end)
--[[
			local li=0
			for line in fp:lines() do
				gui.master.request_redraw=true
				yield_maybe(find)
				li=li+1
				local si=1
				repeat
					local fs,fe=string.find(line,find.word,si,true)
					if fs then
						si=fe+1
--						print("found "..li.." : "..fs.."-"..fe.." : "..file )
						local pp=wpath.parse(file)
						local itdir=find:manifest_dir_item(base_item,pp.path)
						local it={
							parent=itdir,
							name=string.format("%09d/%09d",li,fs),
							path=file,
							file=file,
							text=li.." ("..fs..")",
							mode="file_find",
							bpos={li,fs,fe+1},
							depth=(itdir.depth or 0)+1,
							refresh=function(item) return find:item_refresh(item) end,
						}
						itdir[#itdir+1]=it
						it:refresh()
					end
				until not fs
			end
]]
			if fp then
				fp:close()
			end

		end
--		base_item.text="*"..base_item.word.."*"
--		find:item_refresh(base_item)
--		gui.master.request_redraw=true
		
	end

	return finds
end

--
-- (C) 2025 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local wpath=require("wetgenes.path")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- searches

M.bake=function(oven,finds)

	local finds=finds or {}


	local gui=oven.rebake(oven.modname..".gui")
	local wtreefile=oven.rebake("wetgenes.gamecake.widgets.treefile")


	local find={}
	local find_meta={__index=find}

	finds.oven=oven	
	finds.modname=M.modname

	finds.list={} -- list of finds

	finds.create=function(find)
		local find=find or {}
		setmetatable(find,find_meta)
		finds.list[#finds.list+1]=find
		find.finds=finds
		
		find.path=find.path or ""
		find.text=find.text or ""
		
		return find
	end

	finds.get=function(path,text)
		for i,find in ipairs( finds.list ) do
			if find.path==path and find.text==text then
				return find
			end
		end
	end

-- get the tree item for this search if it exists
	find.get_item=function(find)
		local treefile=gui.master.ids.treefile

		local rekky
		rekky=function(items)
			for _,item in ipairs(items) do
				if	item.mode == "find"    and
					find.path == item.path and 
					find.text == item.text
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
		local item=rekky(treefile.tree_widget.items)

		return item
	end

-- add a tree item for this search
	find.add_item=function(find)
		local treefile=gui.master.ids.treefile

		local dir_item=treefile:add_dir_item(find.path)
		
		local item={
			mode="find",
			path=find.path,
			text=find.text,
		}
		dir_item[#dir_item+1]=item
		treefile:item_sort(dir_item)

		return item
	end

	return finds
end

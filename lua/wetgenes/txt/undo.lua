--[[#lua.wetgenes.txt.undo

(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT

undo / redo code for a text editor with persistence to disk

persistance to disk is in tsv format filename.txt.undo files where a 
.undo is added to the end of the file.

see https://pypi.org/project/linear-tsv/1.0.0/ for tsv format the first 
most column is always a command and the other columns are data needed to 
apply/reverse this command in theory a .undo file is a total history 
and as we should only be appending data (lines at a time even) then a 
file can be recovered from it and it should have limited corruption 
possibilities when things go wrong.

For security reasons this file may have undos removed as a separate 
step. That is to say things that where done / pasted accidentality then 
removed instantly will be purged from its history when performing a 
file save.

You can be save in the knowledge that any information you undo will not 
be saved except as temporary crash safe buffers.

The following need to be escaped with a \ when used in each column.

	\n for newline,
	\t for tab,
	\r for carriage return,
	\\ for backslash.
  
]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstring=require("wetgenes.string")

local wtsv = require("wetgenes.tsv")
local wutf=require("wetgenes.txt.utf")

local cmsgpack=require("cmsgpack")

local zlib=require("zlib")
local inflate=function(d) return ((zlib.inflate())(d)) end
local deflate=function(d) return ((zlib.deflate())(d,"finish")) end




--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local hashnext=function(a,b)
	return ((math.floor((a*31)/16)+b)%0x100000000)
end

local hash
hash=function(...)
	local a=0
	for _,v in ipairs({...}) do
		local t=type(v)
		if t=="string" then -- strings
			for i=1,#s do
				a=hashnext(a,s:byte(i))
			end
		elseif t=="number" then -- numbers
			a=hashnext(a,t)
		elseif t=="table" then -- recurse
			for _,b in ipairs(t) do
				a=hashnext(a,hash(b))
			end
		end
	end
	return a
end
M.hash=hash
M.hashnext=hashnext

-- bind an undo state to a txt state
M.construct=function(undo,txt)
	undo = undo or {}
	
	undo.txt=txt
	txt.undo=undo

	undo.list_reset=function()
		undo.list={}
		undo.index=0
		undo.length=0
		undo.memory=0
	end
	undo.list_reset()
	
	undo.list_set_fromsql=function(rows,idx)
		undo.list_reset()
		for i,row in pairs(rows) do --need to replace txt undos with this data
			undo.list[row.ud]=row.value
			undo.memory=undo.memory+#row.value -- keep running total
		end
		undo.length=#undo.list
		undo.index=idx
	end
	
	undo.list_get=function(index)
		if not index then return end
		local it=undo.list[index]
		it=it and cmsgpack.unpack(inflate(it))
		return it
	end
	
	undo.list_set=function(index,it)
		it=it and deflate(cmsgpack.pack(it))
		undo.list[index]=it
		if index>undo.length then -- added a new one
			undo.length=index
			undo.memory=undo.memory+#it -- keep running total
		end
		if undo.list_hook then
			undo:list_hook("set",index,it) -- call this user function with new/changed data ( write to sqlite database )
		end
	end

	undo.list_push=function(it)
		undo.index=undo.index+1
		undo.list_set(undo.index,it)
		undo.list_trim(undo.index)
	end

	undo.list_trim=function(index)
		index = index or undo.index	--  default to current index
		if index < undo.length then -- need to trim some fat
			for i=undo.length,index+1,-1 do -- remove these
				local it=undo.list[i]
				undo.memory=undo.memory-#it -- keep running total
				undo.list[i]=nil
			end
			undo.length=index
			if undo.list_hook then
				undo:list_hook("trim",index) -- call this user function with new/changed data ( write to sqlite database )
			end
		end
	end

-- ru 1==redo , 2==undo	
	undo.apply=function(index,ru)
		local it=undo.list_get(index)
		
		if     ru==1 then -- redo
		
			local fy,fx=txt.ptr_to_location(it[1])
			local ty,tx=txt.ptr_to_location(it[1]+#it[2])

			local sa=txt.cut(fy,fx,ty,tx)
			txt.mark(fy,fx,fy,fx)
			txt.insert(it[3])

		elseif ru==2 then -- undo

			local fy,fx=txt.ptr_to_location(it[1])
			local ty,tx=txt.ptr_to_location(it[1]+#it[3])

			local sa=txt.cut(fy,fx,ty,tx)
			txt.mark(fy,fx,fy,fx)
			txt.insert(it[2])

		end
	end

	undo.redo=function() -- go forward a step
		if undo.index<undo.length then
			undo.index=undo.index+1
			undo.apply(undo.index,1)
		end
	end

	undo.undo=function() -- go back a step
		if undo.index>0 then
			undo.apply(undo.index,2)
			undo.index=undo.index-1
		end
	end

	undo.redo_all=function() -- fast forwards
		while undo.index<undo.length do
			undo.redo()
		end
	end

	undo.goto=function(index) -- goto a specific index
	end
	
-- remember to insert or delete text at a given (byte) location
	undo.remember=function( insert_string , delete_count , line_idx , char_idx )

		if not line_idx then line_idx=txt.cy end
		if not char_idx then char_idx=txt.cx end
		if not delete_count then delete_count=0 end
		if not insert_string then insert_string="" end

		local fy,fx,ty,tx=txt.rangeget(line_idx,char_idx,delete_count)

		
		local delete_string=txt.copy(fy,fx,ty,tx) or ""

		local ptr=txt.location_to_ptr(fy,fx)
		
--		local py,px=txt.ptr_to_location(ptr)
--print( "PTR" , ptr,py,px )

		if delete_string == "" then -- possible append
			local it = undo.list_get(undo.index)
			if it and ( type(it[1]) == "number" ) then -- an insert packet
				if ( it[1] + #it[3] ) == ptr then -- and we can append to it
				
					-- is it a good idea to append?
					local good_idea=true
					
					if it[3]:find("\n") then -- do not join line
						good_idea=false
					end
					if insert_string:find("\n") then -- do not join lines
						good_idea=false
					end
					if it[3]:find("%s") then -- if old string contains space then only append space
						if insert_string:find("[^%s]") then
							good_idea=false
						end
					end
					if it[3]:find("%w") then -- if old string contains alphanumeric then only append alphanumeric
						if insert_string:find("[^%w]") then
							good_idea=false
						end
					end
					if it[3]:find("%p") then -- if old string contains punctuation then only append punctuation
						if insert_string:find("[^%p]") then
							good_idea=false
						end
					end
					if good_idea then
						it[3]=it[3]..insert_string
						undo.list_set(undo.index,it)
						return
					end
				end
			end
		end

		undo.list_push({ptr,delete_string,insert_string})

	end


-- these functions add to the undo stack and call the txt.* functions

	undo.copy=function()
		return txt.copy()
	end

	undo.cut=function()
		local s=txt.copy()
		if s and #s > 0 then
			undo.remember("",wutf.length(s),txt.fy,txt.fx)
		end
		return txt.cut()
	end

	undo.delete=function()
		undo.remember("",1)
		return txt.delete()
	end
	
	undo.backspace=function()
		undo.remember("",-1)
		return txt.backspace()
	end
	
	undo.insert=function(s)
		undo.remember(s)
		return txt.insert(s)
	end
	
	undo.replace=function(s)
		local o=txt.copy()
		if o and #o > 0 then
			undo.remember(s,wutf.length(o),txt.fy,txt.fx)
			txt.cut()
			return txt.insert(s)
		else
			undo.remember(s)
			return txt.insert(s)
		end
	end

	undo.insert_newline=function()
		local s=txt.get_string(txt.cy) or ""
		local indent=s:match("^([\t ]*)") or ""
		if txt.cx<=#indent then indent="" end -- no auto indent
		undo.remember(indent.."\n")
		return txt.insert_newline(indent)
	end

	undo.insert_char=function(s)
		undo.remember(s)
		return txt.insert_char(s)
	end

	return undo
end

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

	undo.list={}
	undo.length=0
	undo.memory=0
	
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
	end


-- ru 1==redo , 2==undo	
	undo.apply=function(index,ru)
		local it=undo.list_get(index)
	end

	undo.redo=function() -- go forward a step
	end

	undo.undo=function() -- go back a step
	end

	undo.goto=function(index) -- goto a specific index
	end
	
-- remember to insert or delete text at a given (byte) location
	undo.remember=function( insert_string , delete_count , line_idx , char_idx )
		if not line_idx then line_idx=txt.cy end
		local cache=txt.get_cache(line_idx)
		if not char_idx then char_idx=txt.cx end
		if not delete_count then delete_count=0 end
		if not insert_string then insert_string="" end

		local ptr=cache.cb[char_idx] or 0 -- convert to byte offset
		
		local fx,fy,tx,ty=txt.rangeget(char_idx,line_idx,delete_count)
		
		local delete_string=txt.copy(fx,fy,tx,ty) or ""
		ptr=txt.get_cache(fy).start+ptr-1
		
		local py,px=txt.find_line(ptr)

--		print(line_idx,char_idx,delete_count,"ptr",ptr,py,px,"from",fy,fx,(delete_string:gsub('%c','')),(insert_string:gsub('%c','')))

		print(ptr,(delete_string:gsub('%c','')),(insert_string:gsub('%c','')))

	end


-- these functions add to the undo stack and call the txt.* functions

	undo.copy=function()
		return txt.copy()
	end

	undo.cut=function()
		local s=txt.copy()
		if s and #s > 0 then
			undo.remember("",#s,txt.fy,txt.fx)
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
	
	undo.insert_newline=function()
		undo.remember("\n")
		return txt.insert_newline()
	end

	undo.insert_char=function(s)
		undo.remember(s)
		return txt.insert_char(s)
	end

	return txtundo
end

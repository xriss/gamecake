--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstring=require("wetgenes.string")


--[[

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

M.construct=function(txtundo)
	txtundo = txtundo or {}


	return txtundo
end

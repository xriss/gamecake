--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local pack={}

local core=require("wetgenes.pack.core")

local wstr=require("wetgenes.string")

--
-- Read a single member and return it
--
pack.read=function(dats,fmt,off,bigend)
	local datt,len=core.load(dats,{fmt,bigend=bigend},off)
	return datt and datt[1]
end

--
-- Read an array of the same type
--
pack.load_array=function(dats,fmt,off,count,bigend)
	local fmts={bigend=bigend}
	for i=1,count do fmts[i]=fmt end
	return core.load(dats,fmts,off)
end

--
-- write an array of the same type
--
pack.save_array=function(dats,fmt,off,count,bigend)
	local fmts={bigend=bigend}
	for i=1,count do fmts[i]=fmt end
	return core.save(dats,fmts,off)
end

--
-- wrap the core functions with easier to use utility code
--
-- format is "u32","name",...
-- where "u32" is a string id of a data type or the length of a string to read
-- name is the name of the table field to return the data in
--
-- we return a table and the length of the data read in bytes
-- 
--
pack.load=function(dats,fmts,off)

	local fmtt={} -- format field type
	local fmtn={} -- format field name
	
	local len=0
	for i=1,#fmts,2 do -- deinterlace
		local vt=fmts[i]
		local vn=fmts[i+1]
		
		len=len+1
		
		fmtt[len]=vt -- break input into two tables type and name
		fmtn[len]=vn
		
	end
	

-- parse the data	
	local datt,len=core.load(dats,fmtt,off)

-- now we assign the fields to their given names

	local datr={}
	for i,v in ipairs(fmtn) do
		datr[v]=datt[i]
	end

	return datr,len -- we return the parsed data and the length of the data we just read in bytes
end

--
-- the reverse of load
-- we return a string and the length of the data written in bytes, ie the length of the string
--
pack.save=function(data,fmts,off)
	local fmtt={} -- format field type
	local fmtn={} -- format field name
	
	local len=0
	for i=1,#fmts,2 do
		local vt=fmts[i]
		local vn=fmts[i+1]
		
		len=len+1
		
		fmtt[len]=vt -- break input into two tables type and name
		fmtn[len]=vn
		
	end
	
	local datd={} -- place in correct order
	for i,v in pairs(fmtn) do
		datd[i]=data[v]
	end

-- parse the data into a string
	local dat,len=core.save(datd,fmtt,off)

	return dat,len -- we return the parsed data and the length of the data we just read in bytes
end

pack.alloc=function(size)
	return core.alloc(size)
end
pack.sizeof=function(ud)
	return core.sizeof(ud)
end

return pack

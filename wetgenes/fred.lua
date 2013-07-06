--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstr=require("wetgenes.string")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


function M.create(f)

	f.setup= function(f) for i,v in ipairs(f) do if v.setup  then v:setup(f)  end end end
	f.clean= function(f) for i,v in ipairs(f) do if v.clean  then v:clean(f)  end end end
	f.update=function(f) for i,v in ipairs(f) do if v.update then v:update(f) end end end
	f.draw=  function(f) for i,v in ipairs(f) do if v.draw   then v:draw(f)   end end end
	
	f.sort=function(f) table.sort(f,function(a,b) return (a.sort or 0) < (b.sort or 0) end) end

	f.insert=function(f,v) table.insert(f,v) return v end
	
	f.remove=function(f,v) for i,vv in ipairs(f) do if v==vv then table.remove(f,i) return end end end

	return f
end


function M.co(f,c)
	f=f or {}
	f.fred=coroutine.create(c)
	f.yield =function(f,...) coroutine.yield(...) end
	f.resume=function(f,...) coroutine.resume(f.fred,...) end
	return f
end


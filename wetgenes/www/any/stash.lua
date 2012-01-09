-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

local stashdata=require("wetgenes.www.any.stashdata")


-- a stash is a simple long term cache, it lives on disk and survives reboots

module(...)


-----------------------------------------------------------------------------
--
-- clear all stashed data, may fail...
-- everything in the stash should be recreatable
--
-----------------------------------------------------------------------------
function clear(srv,id)
end

-----------------------------------------------------------------------------
--
-- delete id from stash
--
-----------------------------------------------------------------------------
function del(srv,id)
end

-----------------------------------------------------------------------------
--
-- put id in stash
--
-----------------------------------------------------------------------------
function put(srv,id,tab)
end

-----------------------------------------------------------------------------
--
-- get id from stash
--
-----------------------------------------------------------------------------
function get(srv,id)
end

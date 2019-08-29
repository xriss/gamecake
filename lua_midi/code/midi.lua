--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M



local core=require("midi_alsa_core")


local midi=M
local base={}
local meta={}
meta.__index=base

setmetatable(midi,meta)

--[[#lua.wetgenes.midi.create

	m=wmidi.create()

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns a midi object.

]]
-- many options
midi.create=function(...)
	local m={}
	setmetatable(m,meta)
	m[0]=core.create()
	return m
end

--[[#lua.wetgenes.midi.destroy

	m:destroy()

Free the midi and associated memory. This will of course be done 
automatically by garbage collection but you can force it explicitly 
using this function.

]]
base.destroy=function(g)
	return core.destroy(g[0])
end

--[[#lua.wetgenes.midi.clients

	m:clients()

fetch table of clients


]]
base.clients=function(g)
	return core.clients(g[0])
end

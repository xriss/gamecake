-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- This file exists to flag the basedir of the INSERTFILENAMEHERE app
-- it may also contain some lua code that should be run


local apps=apps

local lfs=require("lfs")
local wstr=require("wetgenes.string")

-- pass these args into lunatest
arg={}
local arg=arg
local lunatest=require("lunatest")


module(...)

function start(args)

	for i,v in ipairs( wstr.split_words(args) ) do arg[i]=v end

	lfs.chdir(apps.dir)

	lunatest.suite("test_lanes")

	lunatest.suite("test_lua_al")
	
	lunatest.suite("wetgenes_string")
	lunatest.suite("wetgenes_grd")
	lunatest.suite("wetgenes_grdmap")

	lunatest.run()
	
end

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

require("apps").default_paths() -- default search paths so things can easily be found

-- complain about global vars?
-- I dont think this helps as all the tests are wrapped
local global=require("global")


local apps=require("apps")

local lfs=require("lfs")
local wstr=require("wetgenes.string")

-- pass these args into lunatest
arg={...}
local lunatest=require("lunatest")

lfs.chdir(apps.dir)

if arg[1] then

	lunatest.suite("unit."..arg[1])

else

	lunatest.suite("unit.test_require")
	
	lunatest.suite("unit.test_zip")

--	lunatest.suite("unit.test_lanes")

	lunatest.suite("unit.test_lua_al")
	lunatest.suite("unit.test_lua_freetype")

	lunatest.suite("unit.wetgenes_string")

	lunatest.suite("unit.wetgenes_sod")
	lunatest.suite("unit.wetgenes_ogg")

	lunatest.suite("unit.wetgenes_pack")

	lunatest.suite("unit.wetgenes_speak")

	lunatest.suite("unit.wetgenes_grd")
	lunatest.suite("unit.wetgenes_grdmap")

	lunatest.suite("unit.wetgenes_win")

end

lunatest.run()

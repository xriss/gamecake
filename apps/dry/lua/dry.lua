-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

require("apps").default_paths() -- default search paths so things can easily be found

local apps=require("apps")

local lfs=require("lfs")
local wstr=require("wetgenes.string")

-- pass these args into lunatest
arg={...}
local lunatest=require("lunatest")

lfs.chdir(apps.dir)

if arg[1] then

	lunatest.suite(arg[1])

else

	lunatest.suite("test_zip")

	lunatest.suite("test_lanes")

	lunatest.suite("test_lua_al")
	--lunatest.suite("test_lua_freetype")

	lunatest.suite("wetgenes_string")

	lunatest.suite("wetgenes_sod")
	lunatest.suite("wetgenes_ogg")

	lunatest.suite("wetgenes_pack")

	--lunatest.suite("wetgenes_speak")

	lunatest.suite("wetgenes_grd")
	lunatest.suite("wetgenes_grdmap")

	lunatest.suite("wetgenes_win")

end

lunatest.run()

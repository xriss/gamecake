-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

require("apps").default_paths() -- default search paths so things can easily be found

-- complain about global vars?
-- I dont think this helps as all the tests are wrapped
local global=require("global")


local lfs=require("lfs")
local wstr=require("wetgenes.string")

-- pass these args into lunatest
arg={...}
local lunatest=require("lunatest")

if arg[1] then

	lunatest.suite("unit."..arg[1])

else

	lunatest.suite("unit.test_require")

	lunatest.suite("unit.wetgenes_txt")
	lunatest.suite("unit.wetgenes_txt_diff")
	
	lunatest.suite("unit.test_zip")

	lunatest.suite("unit.wetgenes_path")

--	lunatest.suite("unit.test_lanes")

	lunatest.suite("unit.test_lua_al")
	lunatest.suite("unit.test_lua_freetype")

	lunatest.suite("unit.wetgenes_string")
	lunatest.suite("unit.wetgenes_tardis")

	lunatest.suite("unit.wetgenes_sod")
	lunatest.suite("unit.wetgenes_ogg")

	lunatest.suite("unit.wetgenes_pack")

	lunatest.suite("unit.wetgenes_speak")

	lunatest.suite("unit.wetgenes_grd")
	lunatest.suite("unit.wetgenes_grdmap")
	lunatest.suite("unit.wetgenes_grddiff")
	lunatest.suite("unit.wetgenes_grdpaint")
	lunatest.suite("unit.wetgenes_grdcanvas")

	lunatest.suite("unit.wetgenes_win")

	lunatest.suite("unit.wetgenes_chipmunk")

end

lunatest.run()

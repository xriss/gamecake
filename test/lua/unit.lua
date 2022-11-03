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

	local lfs=require("lfs")

	local units={}
	local path="lua/unit"
	for file in lfs.dir(path) do
		if file:sub(-4)==".lua" then
			units[#units+1] = "unit."..file:sub(1,-5)
		end
	end
	
	table.sort(units)
	for _,name in ipairs(units) do
		lunatest.suite(name)
	end

end

lunatest.run()

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

require("apps").default_paths() -- default search paths so things can easily be found

local apps=require("apps")

local lfs=require("lfs")
local wstr=require("wetgenes.string")

arg={...}

if apps.dir then lfs.chdir(apps.dir) end

if arg[1] then

	require("cmd."..arg[1],unpack(arg))

else

	print("Available test commands are")
	print()
	for fname in lfs.dir("./lua/cmd") do
		if fname:sub(-4)==".lua" then
			print("./cmd "..fname:sub(1,-5))
		end
	end
	print()
	
end

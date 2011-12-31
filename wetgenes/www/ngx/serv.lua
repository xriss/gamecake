-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.ngx.log").log
local debug=require("debug")


local ngx=require("ngx")

module(...)

function serv()
	xpcall(serv2,function(msg,lev)
		log( msg )
		log( debug.traceback() )
	end)
end

function serv2()

	local basic=require("base.basic")

	-- shove this basic functions into the global name space
	-- they will work with the opts to serv this app as needed

	basic.serv(require("wetgenes.www.ngx.srv").new())

end

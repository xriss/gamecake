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
--ngx.log(ngx.ERR,"test321")
--ngx.log(ngx.ERR,tostring(error))

	_G.print("starting srv")
	--log( debug.traceback("test") )
	--error("test",1)

	_G.print("require basic codes")
	local basic=require("base.basic")
	_G.print("required basic codes AOK")

	-- shove this basic functions into the global name space
	-- they will work with the opts to serv this app as needed

	_G.serv_fail=basic.serv_fail

	_G.serv=basic.serv


	_G.serv(require("wetgenes.www.ngx.srv").new())

end

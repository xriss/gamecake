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

local sql=require("sqlite")
local db=assert(sql.open("sqlite/test.sqlite"))
log("sqltest")

--[=[
assert(sql.OK==db:exec([[
	CREATE TABLE t(x INTEGER PRIMARY KEY ASC, y, z);
	INSERT INTO t VALUES(1,2,3);
	INSERT INTO t VALUES(4,5,6);
]]) and db:errmsg())
]=]

assert(sql.OK==db:exec([[
	SELECT * FROM t;
]],
	function(udata,cols,values,names)
		local t={}
		for i=1,cols do t[#t+1]=' ' t[#t+1]=names[i] t[#t+1]=' = ' t[#t+1]=values[i] end
		log(table.concat(t))
		return 0
	end,0) and db:errmsg())

	local basic=require("base.basic")

	-- shove this basic functions into the global name space
	-- they will work with the opts to serv this app as needed

	_G.serv_fail=basic.serv_fail
	_G.serv=basic.serv

	_G.serv(require("wetgenes.www.ngx.srv").new())

end

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log




module(...)
local _M=require(...)
local wdata=require("wetgenes.www.any.data")

default_props=
{
}

default_cache=
{
	base=nil,  --
	func=nil,  -- require(base).func(srv,id) to rebuild this stash
	data={},  -- the data we stashed
}


--------------------------------------------------------------------------------
--
-- allways this kind
--
--------------------------------------------------------------------------------
function kind(srv)
	return "stash"
end

--------------------------------------------------------------------------------
--
-- check that entity has initial data and set any missing defaults
--
--------------------------------------------------------------------------------
function check(srv,ent)

	local c=ent.cache
		
	return ent
end



wdata.set_defs(_M) -- create basic data handling funcs

wdata.setup_db(_M) -- make sure DB exists and is ready



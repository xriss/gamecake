-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.users"]=_M

function login_url(a)

--	log("users.login_url:")
--	return core.login_url(a)

end


function logout_url(a)

--	log("users.logout_url:")
--	return core.logout_url(a)

end

function get_google_user()

--	log("users.get_google_user:")
--	return core.get_google_user()

end

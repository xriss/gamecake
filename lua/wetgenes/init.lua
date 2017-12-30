-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes

	local wetgenes=require("wetgenes")

Simple generic functions that are intended to be useful for all 
wetgenes.* modules.

]]
local M={ modname=(...) } ; package.loaded[M.modname]=M



-----------------------------------------------------------------------------
--[[#lua.wetgenes.safecall

	... = wetgenes.safecall(func,...)

Call a function func(...) wrapped in an xpcall to catch and ignore 
errors, the errors are printed to stderr with a traceback and the 
function returns nil on an error.

So provided the function returns not nil on success then you can still 
tell if the function completed OK. Best to use for things that are OK 
to fail and the rest of the code will work around it.

]]
-----------------------------------------------------------------------------
M.safecall=function(f,...)
	local a={...}
	local r={ xpcall(function() return f(unpack(a)) end,function(err) return "safecall ignoring error: "..debug.traceback(err).."\n" end) }
	if not r[1] then io.stderr:write(r[2]) return end -- print error and return nil on error
	return select(2,unpack(r))
end	

-----------------------------------------------------------------------------
--[[#lua.wetgenes.safewrap

	savefunc = wetgenes.safecall(func)

Wrap a funciton in safecall, so it will never generate errors but can 
be called as normal.

]]
-----------------------------------------------------------------------------
M.safewrap=function(f)
	return function(...)
		return M.safecall(f,...)
	end
end	

-----------------------------------------------------------------------------
--[[#lua.wetgenes.export

	... = wetgenes.export(table,...)

Export multiple names from this table as multiple returns, can be 
used to pull functions out of this module and into locals like so

	local export,lookup,set_env=require("wetgenes"):export("export","lookup","set_env")

Or copy it into other modules to provide them with the same functionality.

	M.lookup=require("wetgenes").lookup

]]
-----------------------------------------------------------------------------

M.export=function(env,...)
	local tab={}
	for i,v in ipairs{...} do
		tab[i]=env[v]
	end
	return unpack(tab)
end

-----------------------------------------------------------------------------
--[[#lua.wetgenes.lookup

	value = wetgenes.lookup(table,...)

Safe recursive lookup within a table that returns nil if any part of 
the lookup is nil so we never cause an error but just return nil. 
This is intended to replace the following sort of code

	a = b and b.c and b.c.d and b.c.d.e

To get e only if all of its parent bits exist and not to cause any 
error if they do not. instead use

	a = lookup(b,"c","d","e")

]]
-----------------------------------------------------------------------------
M.lookup=function(tab,...)
	for i,v in ipairs{...} do
		if type(tab)~="table" then return nil end
		tab=tab[v]
	end
	return tab
end


-----------------------------------------------------------------------------
--[[#lua.wetgenes.set_env

	local _ENV=set_env(new_environment)

Since setfenv is going away in lua 5.2 here is a plan to future 
proof code that wants to control its own environment

Specifically this is for loading functions sabdboxed into a given 
table, which we need to do from time to time.

we set the environment of the code that called us only if setfenv 
exists and we always return the new environment

So the following incantation can be used to change the current environment and 
it should work exactly the same in lua 5.1 or 5.2

	local _ENV=set_env(new_environment)

]]
-----------------------------------------------------------------------------
M.set_env=function(env)
	if setfenv then setfenv(2,env) end
	return env
end


-----------------------------------------------------------------------------
--[[#lua.wetgenes.set_env

	gamecake -e" require('wetgenes').savescripts('./') "

Run the above from the command line.

This will export all the gamecake internal strings into the file system 
it is saved into the current directory so be careful where you run it.

Game Cake checks the files system first so, these files can be modified 
and they will replace the built in versions.

	gamecake -e" require('wetgenes').savescripts('./internal/') "

This is a safer version that will save the files to ./internal/lua/* 
instead of just ./lua/*

]]
-----------------------------------------------------------------------------
M.savescripts=function(basedir)

	assert(basedir, "missing destination path")

	local wbake=require("wetgenes.bake")
	local wgc=require("wetgenes.gamecake.core")
	local names=wgc.list_cache_strings()

	for n,v in pairs(names) do

		local fname
		if "lua/"==v:sub(1,4) then -- these are extra files, eg .glsl code
			fname=v
		else -- these are modules, so need to be turned into files
			fname="lua/"..v:gsub("%.","/")..".lua"
		end

		local data=wgc.get_cache_string(v)
		
		print( "saving" , #data , " bytes as ", basedir..fname )
		
		wbake.create_dir_for_file( basedir..fname )
		wbake.writefile( basedir..fname , data )
		
	end

end

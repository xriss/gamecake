-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes

	local wetgenes=require("wetgenes")

Simple generic functions that are intended to be useful for all 
wetgenes.* modules.

Probably best to cherry pick a few functions you need and export then like so.

	local export,lookup,deepcopy=require("wetgenes"):export("export","lookup","deepcopy")

]]
local M={ modname=(...) } ; package.loaded[M.modname]=M


-----------------------------------------------------------------------------
--[[#lua.wetgenes.rnd64k

	rnd = wetgenes.rnd64k(seed)
	
	r	= rnd()		-- a fraction between 0 and 1 inclusive
	r	= rnd(a)	-- an integer between 1 and a inclusive
	r	= rnd(a,b)	-- an integer between a and b inclusive

a sequence of 65536 numbers starting at seed which should be an integer 
between 0 and 65535 or a string which will be used to generate this 
number.

]]
-----------------------------------------------------------------------------
M.rnd64k=function(seed)
	local bit=bit or require("bit")
	local num=0
	if type(seed)=="number" then num=math.floor(seed)%65536 end
	if type(seed)=="string" then
		for i=1,#seed do
			local n=string.byte(seed,i)	-- ascii code
			local p=2^(((i-1)*3)%16)	-- shift left 3 bits per character
			num=bit.bxor(num,(n*p))		-- overflow upto 7 bits
		end
		num=bit.bxor(num,math.floor(num/65536))%65536	-- xor top and bottom 16 bits
	end
	local roll=function()
		num=(75*(num+1)-1)%65537
		return num
	end
	local rnd=function(a,b)
		if a then
			if not b then b=a a=1 end
			return a+math.floor(0.5+((roll()/65535)*(b-a)))
		end
		return roll()/65535
	end
	return rnd
end	




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

	local export,lookup,deepcopy=require("wetgenes"):export("export","lookup","deepcopy")

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
--[[#lua.wetgenes.deepcopy

	deepcopy(tab)

Create and return a new table containing the same data as the input. If any of
the table values (not keys) are tables then these are also duplicated,
recursively.

If this is called with a value that is not a table then that value is just
returned so it's safe to call on values without checking them.

]]
-----------------------------------------------------------------------------
local deepcopy ; deepcopy=function(orig)
	if type(orig) ~= 'table' then return orig end
	local copy={}
	for k,v in next,orig,nil do
		copy[ deepcopy(k) ] = deepcopy(v)
	end
	return copy
end
M.deepcopy=deepcopy

-----------------------------------------------------------------------------
--[[#lua.wetgenes.deepcompare

	deepcompare(a,b)

Returns true if a==b , this iterates and recurses into tables.

]]
-----------------------------------------------------------------------------
local deepcompare ; deepcompare=function(a,b)
	local ta=type(a)
	local tb=type(b)
	if ta~=tb then return false end -- must be same type

	if ta=="table" then -- iterate (need to do both in case a key only exists on one side)
		for k,av in pairs(a) do
			if not deepcompare(av,b[k]) then return false end
		end
		for k,bv in pairs(b) do
			if not deepcompare(bv,a[k]) then return false end
		end
	else -- simple
		if a~=b then return false end
	end
	return true
end
M.deepcompare=deepcompare

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

	local wpath=require("wetgenes.path")
	local wbake=require("wetgenes.bake")
	local wgc=require("wetgenes.gamecake.core")
	local names=wgc.list_cache_strings()

	for n,fname in pairs(names) do

		local data=wgc.get_cache_string(fname)
		local fpath=wpath.join(basedir,fname)
		print( "saving" , #data , " bytes as ", fpath )
		
		wbake.create_dir_for_file( fpath )
		wbake.writefile( fpath , data )
		
	end

end



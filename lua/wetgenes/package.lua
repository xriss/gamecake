--
-- (C) 2016 kriss@wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.package

	wpackage=require("wetgenes.package")

Some generic lowlevel functions for messing about with how lua works, 
eg how modules are loaded.

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


--[[#lua.wetgenes.package.reload

	local module=wpackage.reload(modulename)

A wrapper for require that forces a very dumb module reload by merging 
a newly loaded module into the old module table if one already exists 
this can easily break everything but should mostly work out of dumb 
luck.

]]
M.reload=function(name)

	if name=="*" then
	
		for n,v in pairs(package.loaded) do -- reload all baked modules
			print("reload",n)
			local suc,err=pcall(function()
				M.reload(n)
			end)
			if not suc then print("IGNORE",err) end
		end
	
	else

		local m=package.loaded[name] -- do we have this module
		
		if type(m)=="table" then -- can attempt a reload
		
			package.loaded[name]=nil -- forget old table
			
			local m2=require(name)
			
			for n,v in pairs(m2) do -- shallow copy new (hopefully mostly code) over old module replacing old values
				m[n]=v
			end
			
			package.loaded[name]=m -- replace with old merged table

		else -- just a normal require

			m=require(name)
			
		end

		return m

	end
	
end


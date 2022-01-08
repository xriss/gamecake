--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- maintain a set of defaults for easier widget creation

M.create=function()
	local def={}

	def.classes={}

	def.set=function(it)
		def.classes[ it.class ]=def.classes[ it.class ] or {}
		for n,v in pairs(it) do
			def.classes[ it.class ][n]=v
		end
	end
	
	def.reset=function(it)
		def.classes[ it.class ]=nil
		def.set(it)
	end

	def.copy=function(it)
		for n,v in pairs( it.class and def.classes[it.class] or {} ) do
			if type(it[n])=="nil" then
				it[n]=v
			end
		end
		for n,v in pairs( def.classes["*"] or {} ) do
			if type(it[n])=="nil" then
				it[n]=v
			end
		end
		return it
	end

	def.add=function(parent,it)
		return parent:add( def.copy(it) )
	end

	def.add_border=function(parent,it)
		return parent:add_border( def.copy(it) )
	end

	return def
end

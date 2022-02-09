--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.widgets.defs

Helpers to define defaults for each class of widget 

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- maintain a set of defaults for easier widget creation

--[[#lua.wetgenes.gamecake.widgets.defs.create

]]
M.create=function(grid_size)

	grid_size=grid_size or 1

	local def={}

	def.classes={}

--[[#lua.wetgenes.gamecake.widgets.defs.set

]]
	def.set=function(it)
		def.classes[ it.class ]=def.classes[ it.class ] or {}
		for n,v in pairs(it) do
			def.classes[ it.class ][n]=v
		end
	end
	
--[[#lua.wetgenes.gamecake.widgets.defs.reset

]]
	def.reset=function(it)
		def.classes[ it.class ]=nil
		def.set(it)
	end

--[[#lua.wetgenes.gamecake.widgets.defs.copy

]]
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
		for _,n in ipairs({"px","py","hx","hy"}) do
			if it[n] then it[n]=it[n]*grid_size end -- auto scale these numbers by grid_size
		end
		for i,v in ipairs(it) do def.copy(v) end -- and children
		return it
	end

--[[#lua.wetgenes.gamecake.widgets.defs.add

]]
	def.add=function(parent,it)
		return parent:add( def.copy(it) )
	end

--[[#lua.wetgenes.gamecake.widgets.defs.add_border

]]
	def.add_border=function(parent,it)
		return parent:add_border( def.copy(it) )
	end

	return def
end

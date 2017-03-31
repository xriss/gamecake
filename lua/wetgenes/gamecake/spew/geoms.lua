--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local function dprint(a) print(wstr.dump(a)) end

-- helper functions for an array of geoms, EG a scene.

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,geoms)
	geoms=geoms or {}
	geoms.modname=M.modname

	geoms.meta={__index=geoms}
	geoms.new=function(it) it=it or {} setmetatable(it,geoms.meta) return it end

-- call a geom function on each geom in the geoms array
	geoms.call=function(its,name,...)
		for i,it in ipairs(its) do
			it[name](it,...)
		end
		return its
	end


	return geoms
end


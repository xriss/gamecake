--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-- render some geoms to an image
M.fill=function(oven,geom)

	local views=oven.rebake("wetgenes.gamecake.views")
	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")


	local gl=oven.gl

	geom.render=function(graph)

		local fbo=framebuffers.create(it.hx,it.hy,0)
		local view=views.create({
			mode="fbo",
			fbo=it.fbo,
			vx=it.hx,
			vy=it.hy,
			vz=it.hy*4,
		})


		return graph
	end

	return geom
end


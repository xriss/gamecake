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

-- we DO NOT have OpenGL access here
M.meta={__index=M}
M.new=function(it) it=it or {} setmetatable(it,M.meta) return it end

-- call a geom function on each geom in the geoms array

M.call=function(its,name,...)
	for i,it in ipairs(its) do
		it[name](it,...)
	end
	return its
end

M.push=function(its,it)
	its[#its+1]=it
	return its
end

M.reset=function(its)
	for idx=#its,1,-1 do
		its[idx]=nil
	end
	its.scenes={}
	its.mats={}
end


M.bake=function(oven,geoms)

	local geom=oven.rebake("wetgenes.gamecake.spew.geom")

	local gl=oven.gl

	geoms=geoms or {}
	for n,v in pairs(M) do geoms[n]=v end
	geoms.modname=M.modname

-- we DO have OpenGL access here
	geoms.meta={__index=geoms}
	geoms.new=function(it) it=it or {} setmetatable(it,geoms.meta) return it end
	
	geoms.draw=function(its,progname,cb,modes)
	
		local draw
		
		draw=function(its)

			gl.PushMatrix()

			if its.trs then
				local trs=its.trs
			
--				print(trs[1],trs[2],trs[3])

				gl.Scale( trs[8] , trs[9] , trs[10] )
				gl.MultMatrix( tardis.q4_to_m4( { trs[4] , trs[5] , trs[6] , trs[7] } ) )
				gl.Translate( trs[1] , trs[2] , trs[3])

			elseif its.matrix then

--				gl.MultMatrix( its.matrix )

			end

			if its.mesh then
				geom.draw(its.mesh,progname,cb,modes)
			end
			
			for i,v in ipairs(its) do
				draw(v)
			end
			
			gl.PopMatrix()

		end

		draw(its.scene)
	
	end

	return geoms
end


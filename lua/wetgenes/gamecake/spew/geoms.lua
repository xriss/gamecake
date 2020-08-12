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
	
	geoms.reset_anim=function(objs)

		for nidx=1,#objs.nodes do
			local node=objs.nodes[nidx]
			if node.reset_matrix then
				for i,v in ipairs(node.reset_matrix) do
					node.matrix[i]=v
				end
			elseif node.reset_trs then
				for i,v in ipairs(node.reset_trs) do
					node.trs[i]=v
				end
			end
		end

	end

	geoms.animate=function(objs)
	
		if not objs.anim then return end

		local anim=objs.anim

		anim.length=anim.max-anim.min
		if anim.length>0 then
			anim.time=(anim.time or 0)%anim.length
		end
		
		local key_idx=1
		local key_blend=0
		
		for idx=2,#anim.keys do
			local tb=anim.keys[idx-1]
			local ta=anim.keys[idx]
			if tb<=anim.time and ta>=anim.time then -- found
				key_idx=idx-1
				local d=ta-tb
				if d<=0 then d=1 end
				key_blend=( anim.time-tb / d )
				if key_blend>1 then key_blend=1 end
				if key_blend<0 then key_blend=0 end
			end
		end


		for vidx=1,#anim.values do
			local value=anim.values[vidx]
			local node=objs.nodes[value.node]
			
			if value.path=="translation" then

				local i=key_idx*3-2
				local vb={ value.data[i  ] or 0, value.data[i+1] or 0, value.data[i+2] or 0}
				local va={ value.data[i+3] or 0, value.data[i+4] or 0, value.data[i+5] or 0}
				local xb=1-key_blend
				node.trs[1]=vb[1]*xb + va[1]*key_blend
				node.trs[2]=vb[2]*xb + va[2]*key_blend
				node.trs[3]=vb[3]*xb + va[3]*key_blend

			elseif value.path=="rotation" then

				local i=key_idx*4-3
				local vb={ value.data[i  ] or 0, value.data[i+1] or 0, value.data[i+2] or 0, value.data[i+3] or 0}
				local va={ value.data[i+4] or 0, value.data[i+5] or 0, value.data[i+6] or 0, value.data[i+7] or 0}
				local xb=1-key_blend
				local q=tardis.q4.new(
					vb[1]*xb + va[1]*key_blend ,
					vb[2]*xb + va[2]*key_blend ,
					vb[3]*xb + va[3]*key_blend ,
					vb[4]*xb + va[4]*key_blend ):normalize()
				node.trs[4]=q[1]
				node.trs[5]=q[2]
				node.trs[6]=q[3]
				node.trs[7]=q[4]

--print( anim.time , q[1] , q[2] , q[3] , q[4] )

			elseif value.path=="scale" then

				local i=key_idx*3-2
				local vb={ value.data[i  ] or 0, value.data[i+1] or 0, value.data[i+2] or 0}
				local va={ value.data[i+3] or 0, value.data[i+4] or 0, value.data[i+5] or 0}
				local xb=1-key_blend
				node.trs[ 8]=vb[1]*xb + va[1]*key_blend
				node.trs[ 9]=vb[2]*xb + va[2]*key_blend
				node.trs[10]=vb[3]*xb + va[3]*key_blend

			end

		end

	end

	geoms.prepare=function(its)
	
		local stack=tardis.m4_stack()
	
		local prepare
		prepare=function(its)

			stack.push()

			local trs=its.trs

			if trs then

				stack.translate( trs[1] , trs[2] , trs[3])
				stack.rotate( { trs[4] , trs[5] , trs[6] , trs[7] } )
				stack.scale( trs[8] , trs[9] , trs[10] )

			elseif its.matrix then

				stack.product( its.matrix )

			end
			
			its.world=stack.save()
			
			if its.inverse then
--				its.bone=its.inverse:product(its.world,tardis.m4.new())
				its.bone=its.world:product(its.inverse,tardis.m4.new())
			end
			
			for i,v in ipairs(its) do
				prepare(v)
			end
			
			stack.pop()

		end

		prepare(its.scene)

	end

	geoms.update=function(its)
		geoms.animate(its)
		geoms.prepare(its)
	end

	geoms.draw=function(its,progname,cb,modes)
	
		local draw
		
		draw=function(its)

			if its.mesh then
				gl.PushMatrix()
				gl.MultMatrix( its.world )
				geom.draw(its.mesh,progname,cb,modes)
				gl.PopMatrix()
			end

			for i,v in ipairs(its) do
				draw(v)
			end

		end

		draw(its.scene)
	
	end

	geoms.draw_bones=function(its,progname,cb,modes)
	
		local mesh=geom.hexahedron():adjust_scale(1/4)
	
		local draw
		
		draw=function(its)

			if its.bone then
				gl.PushMatrix()
				gl.MultMatrix( its.world )
				geom.draw(mesh,progname,cb,modes)
				gl.PopMatrix()
			end

			for i,v in ipairs(its) do
				draw(v)
			end

		end

		draw(its.scene)
	
	end

	return geoms
end


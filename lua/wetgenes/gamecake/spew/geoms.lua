--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

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

--[[

set the node TRS from the anim data for a given key index and then 
build the local transform matrix for each node.

Then build array of floats that represent a local transform matrix of 
each bone and can be stored in a texture. 12 floats per matrix so the 
GL matrix order is transposed and the final v4 should be assumed to be 
{0,0,0,1}

]]
	geoms.set_anim_frame=function(objs,key_idx,bones)

		local anim=objs.anim

		for vidx=1,#anim.values do
			local value=anim.values[vidx]
			local node=objs.nodes[value.node]
			
			if value.path=="translation" then

				local i=key_idx*3-2
				node.trs[1]=value.data[i  ] or 0
				node.trs[2]=value.data[i+1] or 0
				node.trs[3]=value.data[i+2] or 0

			elseif value.path=="rotation" then

				local i=key_idx*4-3
				node.trs[4]=value.data[i  ] or 0
				node.trs[5]=value.data[i+1] or 0
				node.trs[6]=value.data[i+2] or 0
				node.trs[7]=value.data[i+3] or 0

			elseif value.path=="scale" then

				local i=key_idx*3-2
				node.trs[ 8]=value.data[i  ] or 0
				node.trs[ 9]=value.data[i+1] or 0
				node.trs[10]=value.data[i+2] or 0

			end

		end
		
		local prepare ; prepare=function(its)

			its.matrix=tardis.m4.new():identity()

			local trs=its.trs
			if trs then

				its.matrix:translate( trs[1] , trs[2] , trs[3])
				its.matrix:rotate( { trs[4] , trs[5] , trs[6] , trs[7] } )
				its.matrix:scale( trs[8] , trs[9] , trs[10] )

			end
						
			for i,v in ipairs(its) do
				prepare(v)
			end

		end
		prepare(objs.scene)

		bones=bones or {}
		local skin=objs.skins[1] -- assume only one boned character
		if skin then
			local b=0
			for i,v in ipairs(skin.nodes) do
				local bone=v.matrix or M4()

				bones[b+1]=bone[1]
				bones[b+2]=bone[5]
				bones[b+3]=bone[9]
				bones[b+4]=bone[13]

				bones[b+5]=bone[2]
				bones[b+6]=bone[6]
				bones[b+7]=bone[10]
				bones[b+8]=bone[14]

				bones[b+9]=bone[3]
				bones[b+10]=bone[7]
				bones[b+11]=bone[11]
				bones[b+12]=bone[15]

				b=b+12
			end
		end
		
		return bones

	end
	
--[[

get the tweak and reset matrix of the bones

]]
	geoms.get_anim_tweaks=function(objs,bones)

		bones=bones or {}
		local skin=objs.skins[1] -- assume only one boned character
		if skin then
			local b=0
			for i,v in ipairs(skin.nodes) do
				local m=M4():identity()

				if v.tweak then
					local trs=v.tweak
					m:translate( trs[1] , trs[2] , trs[3] )
					m:rotate( { trs[4] , trs[5] , trs[6] , trs[7] } )
					m:scale( trs[8] , trs[9] , trs[10] )
				end

				bones[b+1]=m[1]
				bones[b+2]=m[5]
				bones[b+3]=m[9]
				bones[b+4]=m[13]

				bones[b+5]=m[2]
				bones[b+6]=m[6]
				bones[b+7]=m[10]
				bones[b+8]=m[14]

				bones[b+9]=m[3]
				bones[b+10]=m[7]
				bones[b+11]=m[11]
				bones[b+12]=m[15]

				b=b+12
			end

			for i,v in ipairs(skin.nodes) do
				local bone=v.inverse or M4()

				bones[b+1]=bone[1]
				bones[b+2]=bone[5]
				bones[b+3]=bone[9]
				bones[b+4]=bone[13]

				bones[b+5]=bone[2]
				bones[b+6]=bone[6]
				bones[b+7]=bone[10]
				bones[b+8]=bone[14]

				bones[b+9]=bone[3]
				bones[b+10]=bone[7]
				bones[b+11]=bone[11]
				bones[b+12]=bone[15]

				b=b+12
			end

		end
		
		return bones

	end

--[[

get skeleton glsl code

]]
	geoms.get_skeleton_glsl=function(objs)
		local ss={}
		local p=function(s) ss[#ss+1]=s end
		
		local skin=objs.skins[1] -- assume only one boned character

		local r ; r=function(idx)
			local node=skin.nodes[idx]
			if node.pidx and skin.nodes[node.pidx] then
				return r(node.pidx).."*texbone("..(idx-1)..",frame)"
			else
				return "texbone("..(idx-1)..",frame)"
			end
		end

p([[
mat4 skeleton(int bidx,int frame)
{
	switch(bidx)
	{
]])

		for i,v in ipairs(skin.nodes) do
			v.sidx=i
		end
		local prepare ; prepare=function(its,pidx)
			its.pidx=pidx
			for i,v in ipairs(its) do
				prepare(v,its.sidx)
			end
		end

		prepare(objs.scene)
		
		for i,v in ipairs(skin.nodes) do

p([[
		case ]]..(i-1)..[[: return ]]..r(i)..[[;
]])
		end

p([[
		default: return texbone(bidx,frame);
	}
}
]])

	
		return table.concat(ss)
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
		
		anim.frame=key_idx


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

		for i,node in ipairs( objs.nodes ) do
			if node.trs and node.tweak then
				node.trs[ 1]=node.trs[ 1]+node.tweak[ 1]
				node.trs[ 2]=node.trs[ 2]+node.tweak[ 2]
				node.trs[ 3]=node.trs[ 3]+node.tweak[ 3]

				local qa=Q4( node.trs[4] , node.trs[5] , node.trs[6] , node.trs[7] )
				local qb=Q4( node.tweak[4] , node.tweak[5] , node.tweak[6] , node.tweak[7] )				
				tardis.q4_product_q4(qa,qb)

				node.trs[ 4]=qa[1]
				node.trs[ 5]=qa[2]
				node.trs[ 6]=qa[3]
				node.trs[ 7]=qa[4]

				node.trs[ 8]=node.trs[ 8]*node.tweak[8]
				node.trs[ 9]=node.trs[ 9]*node.tweak[9]
				node.trs[10]=node.trs[10]*node.tweak[10]
			end
		end
	end

	geoms.prepare=function(objs)
	
		local stack=tardis.m4_stack()
	
		local prepare ; prepare=function(its)

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
				its.bone=its.world:product(its.inverse,tardis.m4.new())
			end
			
			for i,v in ipairs(its) do
				prepare(v)
			end
			
			stack.pop()

		end

		prepare(objs.scene)

	end

	geoms.update=function(objs)
		geoms.animate(objs)
		geoms.prepare(objs)
	end

	geoms.draw=function(objs,progname,cb,modes)
	
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

		draw(objs.scene)
	
	end

	geoms.draw_bones=function(objs,progname,cb,modes)
	
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

		draw(objs.scene)
	
	end

	return geoms
end


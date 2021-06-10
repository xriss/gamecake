--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- Signed Distance Function or Field ( well technically bounds )

local wstr=require("wetgenes.string")

local wgeom=require("wetgenes.gamecake.spew.geom")

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")


local function dprint(a) print(wstr.dump(a)) end


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.export=function(env,...)
	local tab={...} ; for i=1,#tab do tab[i]=env[ tab[i] ] end
	return unpack(tab)
end


--[[

sdf  function(v3) that returns a signed distance
siz  x,y,z bounding box
rez  edge length of each voxel 

returns a geom

]]
M.sdf_to_geom=function(_sdf,siz,rez)

	local sdf=function(p)
		local r = (p:abs(V3())-siz):max()
		local b = _sdf(p)
		return math.max( r , b )
	end
--	sdf=_sdf

	local obj=wgeom.new()

	siz:abs()

	local h = siz:ceil(siz.new())/rez

--	local cache={}

	local dist=function(x,y,z)
	
		local r
--[[
		if cache[x] then
			if cache[x][y] then
				r=cache[x][y][z]
				if r then return r end
			else
				cache[x][y]={}
			end
		else
			cache[x]={}
			cache[x][y]={}
		end
]]
		r=sdf( V3( (x+0.5)*rez , (y+0.5)*rez , (z+0.5)*rez ) )
--		cache[x][y][z]=r
		return r
	end

-- iterate over each cell
	for z=-h[3],h[3]-1 do
		for y=-h[2],h[2]-1 do
			for x=-h[1],h[1]-1 do

				local r=dist(x,y,z)
				if r<=0 then -- solid so check edge and build a polygon if they are not solid

					if dist(x-1,y,z) >0 then
						local vi=#obj.verts
						obj.polys[#obj.polys+1]={vi+1,vi+2,vi+3,vi+4}
						obj.verts[vi+1]={( x   )*rez,( y+1 )*rez,( z   )*rez}
						obj.verts[vi+2]={( x   )*rez,( y   )*rez,( z   )*rez}
						obj.verts[vi+3]={( x   )*rez,( y   )*rez,( z+1 )*rez}
						obj.verts[vi+4]={( x   )*rez,( y+1 )*rez,( z+1 )*rez}
					end
					if dist(x+1,y,z) >0 then
						local vi=#obj.verts
						obj.polys[#obj.polys+1]={vi+1,vi+2,vi+3,vi+4}
						obj.verts[vi+1]={( x+1 )*rez,( y   )*rez,( z   )*rez}
						obj.verts[vi+2]={( x+1 )*rez,( y+1 )*rez,( z   )*rez}
						obj.verts[vi+3]={( x+1 )*rez,( y+1 )*rez,( z+1 )*rez}
						obj.verts[vi+4]={( x+1 )*rez,( y   )*rez,( z+1 )*rez}
					end
					if dist(x,y-1,z) >0 then
						local vi=#obj.verts
						obj.polys[#obj.polys+1]={vi+1,vi+2,vi+3,vi+4}
						obj.verts[vi+1]={( x   )*rez,( y   )*rez,( z   )*rez}
						obj.verts[vi+2]={( x+1 )*rez,( y   )*rez,( z   )*rez}
						obj.verts[vi+3]={( x+1 )*rez,( y   )*rez,( z+1 )*rez}
						obj.verts[vi+4]={( x   )*rez,( y   )*rez,( z+1 )*rez}
					end
					if dist(x,y+1,z) >0 then
						local vi=#obj.verts
						obj.polys[#obj.polys+1]={vi+1,vi+2,vi+3,vi+4}
						obj.verts[vi+1]={( x+1 )*rez,( y+1 )*rez,( z   )*rez}
						obj.verts[vi+2]={( x   )*rez,( y+1 )*rez,( z   )*rez}
						obj.verts[vi+3]={( x   )*rez,( y+1 )*rez,( z+1 )*rez}
						obj.verts[vi+4]={( x+1 )*rez,( y+1 )*rez,( z+1 )*rez}
					end
					if dist(x,y,z-1) >0 then
						local vi=#obj.verts
						obj.polys[#obj.polys+1]={vi+1,vi+2,vi+3,vi+4}
						obj.verts[vi+1]={( x+1 )*rez,( y   )*rez,( z   )*rez}
						obj.verts[vi+2]={( x   )*rez,( y   )*rez,( z   )*rez}
						obj.verts[vi+3]={( x   )*rez,( y+1 )*rez,( z   )*rez}
						obj.verts[vi+4]={( x+1 )*rez,( y+1 )*rez,( z   )*rez}
					end
					if dist(x,y,z+1) >0 then
						local vi=#obj.verts
						obj.polys[#obj.polys+1]={vi+1,vi+2,vi+3,vi+4}
						obj.verts[vi+1]={( x   )*rez,( y   )*rez,( z+1 )*rez}
						obj.verts[vi+2]={( x+1 )*rez,( y   )*rez,( z+1 )*rez}
						obj.verts[vi+3]={( x+1 )*rez,( y+1 )*rez,( z+1 )*rez}
						obj.verts[vi+4]={( x   )*rez,( y+1 )*rez,( z+1 )*rez}
					end
				
				end
				
			end
		end
	end

	print(#obj.verts,#obj.polys)
	obj:merge_points_by_pos()
	print(#obj.verts,#obj.polys)

	local tiny=1/256
	local shrink_wrap=function()
		for iv,vv in ipairs(obj.verts) do
			local va=V3(vv[1],vv[2],vv[3])
			local b=sdf(va)
			local n=V3(b)
			n[1]=n[1]-sdf(V3(vv[1]-tiny,vv[2],vv[3]))
			n[2]=n[2]-sdf(V3(vv[1],vv[2]-tiny,vv[3]))
			n[3]=n[3]-sdf(V3(vv[1],vv[2],vv[3]-tiny))
			n:normalize()
			local s=-1*b
			vv[1]=vv[1]+(n[1]*s)
			vv[2]=vv[2]+(n[2]*s)
			vv[3]=vv[3]+(n[3]*s)
		end
	end

	local spring_length=0
	local spring_count=0

	local measure_springs=function()
		spring_length=0
		spring_count=0
		for ip,vp in ipairs(obj.polys) do
			for ia=1,#vp-1 do
				for ib=ia+1,#vp do
					local s=1
					if ib>ia+1 and ib<#vp then s=1.41421356237 end -- diagonal
					local va=obj.verts[ vp[ia] ]
					local vb=obj.verts[ vp[ib] ]
					local vc=V3( va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] )
					spring_length=spring_length+(vc:len()/s)
					spring_count=spring_count+1
				end
			end
		end
		spring_length=spring_length/spring_count -- average length
	end

	local minlen=rez/65536 --  a tiny length

	local apply_springs=function()
		local t=spring_length/4
		for iv,vv in ipairs(obj.verts) do -- zero and push out by normals a little
			vv.spring=V3(vv[4]*t,vv[5]*t,vv[6]*t)
			vv.spring_count=1
		end
		for ip,vp in ipairs(obj.polys) do -- add
			for ia=1,#vp-1 do
				for ib=ia+1,#vp do
					local va=obj.verts[ vp[ia] ]
					local vb=obj.verts[ vp[ib] ]
					local vc=V3( va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] )
					local vc_len=vc:len()
					if vc_len>minlen then -- must have some length or we ignore it
						vc:normalize() -- calculate normal
						local s=1
						if ib>ia+1 and ib<#vp then s=1.41421356237 end -- diagonal
						local d=(spring_length*s-vc_len)/1

						va.spring[1]=va.spring[1]+vc[1]*d
						va.spring[2]=va.spring[2]+vc[2]*d
						va.spring[3]=va.spring[3]+vc[3]*d
						va.spring_count=va.spring_count+1
						
						vb.spring[1]=vb.spring[1]+vc[1]*-d
						vb.spring[2]=vb.spring[1]+vc[2]*-d
						vb.spring[3]=vb.spring[1]+vc[3]*-d
						vb.spring_count=vb.spring_count+1
					end
				end
			end
		end
		for iv,vv in ipairs(obj.verts) do -- apply
			vv[1]=vv[1]+(vv.spring[1]/vv.spring_count)
			vv[2]=vv[2]+(vv.spring[2]/vv.spring_count)
			vv[3]=vv[3]+(vv.spring[3]/vv.spring_count)
			vv.spring=nil
			vv.spring_count=nil
		end
	end

	local split_quads=function()
		local iplen=#obj.polys
		for ip=1,iplen do
			local vp=obj.polys[ip]
			if #vp==4 then -- choose split along shortest diagonal
				local va=obj.verts[ vp[1] ]
				local vb=obj.verts[ vp[3] ]
				local vc=V3( va[1]-vb[1] , va[2]-vb[2] , va[3]-vb[3] )

				local vd=obj.verts[ vp[2] ]
				local ve=obj.verts[ vp[4] ]
				local vf=V3( vd[1]-ve[1] , vd[2]-ve[2] , vd[3]-ve[3] )

				if vc:len() < vf:len() then
					obj.polys[ #obj.polys+1 ]={ vp[3] , vp[4] , vp[1] }
					vp[4]=nil
				else
					obj.polys[ #obj.polys+1 ]={ vp[2] , vp[3] , vp[4] }
					vp[3]=vp[4]
					vp[4]=nil
				end
				
			end
		end
	end

	obj:build_normals()
	shrink_wrap()

	for i=1,2 do
		measure_springs()
		apply_springs()
		obj:build_normals()
		shrink_wrap()
	end


	split_quads()

	obj:build_normals()

	for iv,vv in ipairs(obj.verts) do
		vv[7]=0.5
		vv[8]=(14.5)/16
	end


	return obj
end

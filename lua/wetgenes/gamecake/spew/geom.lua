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

-- verts are just a bunch of numbers rather than names
-- (hopefully a mild step towards optimisation without too much pain)
-- .plan is to turn this into large native memory arrays for speed at a later point

-- v[1,2,3]			== xyz position
-- v[4,5,6]			== xyz normal
-- v[7,8]			== uv texture coords
-- v[9,10,11,12]	== texture tangent(xyz) and sign(+1 or -1) of bitangent for simple tangent space matrix reconstruction
-- v[13,14,15,16]	== array of combined bone id and weights. integer part is the bone id and 1-frac is the bone weight

-- v.mask			== mask value for selecting verts for transforms and special display

-- all of these values may actually be nil and should default to 0 if they are

-- polygons or lines are just a list of vertex indexes, and a material index in p.mat

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-- vertex data
	M.idxs=	{
				POS_X= 1, POS_Y= 2, POS_Z= 3,
				NRM_X= 4, NRM_Y= 5, NRM_Z= 6,
				TEX_U= 7, TEX_V= 8,
				TAN_X= 9, TAN_Y=10, TAN_Z=11, TAN_W=12,
				BNE_A=13, BNE_B=14, BNE_C=15, BNE_D=16
	}


-- we DO NOT have OpenGL access here
	M.meta={__index=M}
	M.new=function(it) it=it or {} setmetatable(it,M.meta) return it:reset() end
	
	M.reset=function(it)
		it.verts={}
		it.mats={}
		it.polys={}
		it.mats={}
		return it
	end
	
	M.duplicate=function(it)
		return M.copy(it,M.new())
	end

	M.copy=function(src,dst)
-- reset
		dst:reset()
		dst.name=src.name
		if dst.clear_predraw then dst:clear_predraw() end
-- copy mats
		for im = 1 , #src.mats do local mat=src.mats[im]
			local m={}
			m.name=mat.name
			m.diffuse={unpack(mat.diffuse or {})}
			m.specular={unpack(mat.specular or {})}
			m.shininess={unpack(mat.shininess or {})}
			dst.mats[ im ]=m
		end
-- copy verts
		for iv = 1 , #src.verts do local vert=src.verts[iv]
			local v={unpack(vert)}
			dst.verts[ iv ]=v
		end
--copy polys		
		for iv = 1 , #src.polys do local poly=src.polys[iv]
			local p={unpack(poly)}
			p.mat=poly.mat
			dst.polys[ iv ]=p
		end
		return dst
	end

	-- remove vertexes and polygons that match these bones ( array of booleans where false will remove this bone )
	M.filter_by_bones=function(it,bones)

-- work out which vertexes we will remove
		local vmap={}
		for i=1,#it.verts do local v=it.verts[i]
			vmap[i]=true
			for bi=1,4 do
				local b=v[12+bi] or 0
				local bidx=math.floor(b)
				if bidx>0 then
					if not bones[bidx] then vmap[i]=false end -- we will remove this vertex
				end
			end
		end

-- work out new vertex indexes
		local idx=1
		for i=1,#vmap do
			if vmap[i] then
				vmap[i]=idx
				idx=idx+1
			end
		end

-- copy good verts
		local vs={}
		for iv = 1 , #it.verts do local vert=it.verts[iv]
			if vmap[iv] then
				local v={unpack(vert)}
				vs[ vmap[iv] ]=v
			end
		end
		it.verts=vs
		
--copy good polys
		local ps={}
		for iv = 1 , #it.polys do local poly=it.polys[iv]
			local ok=true
			for i=1,#poly do if not vmap[ poly[i] ] then ok=false end end
			if ok then
				local p={}
				for i=1,#poly do p[ i ]=vmap[ poly[i] ] end
				p.mat=poly.mat
				ps[#ps+1]=p
			end
		end
		it.polys=ps

		return it
	end

	M.merge_from=function(dst,src)
-- reset cache
		if dst.clear_predraw then
			dst:clear_predraw()
		end
		
		local remat={}
-- copy mats
		for im = 1 , #src.mats do local mat=src.mats[im]
			local m
			for om = 1 , #dst.mats do local dm=dst.mats[om]
				if not dm.idx then dm.idx=om end -- make sure we have dst idx
				if mat.name and dm.name==mat.name then
					m=dm -- merge into this mat
				end
			end
			if not m then
				m={} -- new material
				m.idx=#dst.mats+1
				dst.mats[ m.idx ]=m
			end
			remat[im]=m.idx
			
			for n,v in pairs(mat) do
				if n~="idx" then
					m[n]=v
				end
			end
		end
-- copy verts
		local bv=#dst.verts
		for iv = 1 , #src.verts do local vert=src.verts[iv]
			local v={unpack(vert)}
			dst.verts[ bv+iv ]=v
		end
--copy polys		
		local bp=#dst.polys
		for iv = 1 , #src.polys do local poly=src.polys[iv]
			local p={unpack(poly)}
			for i,v in ipairs(p) do p[i]=v+bv end
			p.mat=poly.mat and remat[poly.mat] or poly.mat
			dst.polys[ bp+iv ]=p
		end
		return dst
	end

	M.adjust_vertex_by_m4_and_m3=function(v,m4,m3)
		local t=V4(v[1],v[2],v[3],1)
		t:product(m4)
		v[1]=t[1]
		v[2]=t[2]
		v[3]=t[3]
		if v[4] then -- normals
			local t=V4(v[4],v[5],v[6],1)
			t:product(m3)
			v[4]=t[1]
			v[5]=t[2]
			v[6]=t[3]
		end
		if v[9] then -- tangents
			local t=V4(v[9],v[10],v[11],1)
			t:product(m3)
			v[9]=t[1]
			v[10]=t[2]
			v[11]=t[3]
		end
	end

	-- apply an m4 transform to all vertexs and normals/tangents
	M.adjust_by_m4=function(it,m4)
		local m3=M4(m4):m3() -- get rotation/scale only
		local vs=it.verts
		for i=1,#vs do local v=vs[i]
			M.adjust_vertex_by_m4_and_m3(v,m4,m3)
		end
		return it
	end

	-- apply an array of m4 bones transform to all vertexs and normals/tangents
	M.adjust_by_bones=function(it,bones)

		local vs=it.verts
		for i=1,#vs do local v=vs[i]

			local m4
			for bi=1,4 do
				local b=v[12+bi] or 0
--				print(b)
				local bidx=math.floor(b)
				local bwgt=1-(b-bidx)
				local bb=(bidx-1)*16
				local bone=M4(
					bones[bb+1],bones[bb+2],bones[bb+3],bones[bb+4] ,
					bones[bb+5],bones[bb+6],bones[bb+7],bones[bb+8] ,
					bones[bb+9],bones[bb+10],bones[bb+11],bones[bb+12] ,
					bones[bb+13],bones[bb+14],bones[bb+15],bones[bb+16] )
				if bidx>0 and bwgt>0 and bwgt<=1 then
					if not m4 then
						m4=M4(bone):scalar(bwgt)
					else
						m4:add( M4(bone):scalar(bwgt) )
					end
				else
					break
				end
			end
			if not m4 then m4=M4() end
			local m3=M4(m4):m3() -- get rotation/scale only
			

			M.adjust_vertex_by_m4_and_m3(v,m4,m3)
		end
		return it
	end

	-- scale the geom
	M.adjust_scale=function(it,s)
		s=s or 1
		local vs=it.verts
		for i=1,#vs do local v=vs[i]
			v[1]=v[1]*s
			v[2]=v[2]*s
			v[3]=v[3]*s
		end
		return it
	end
	
	-- position the geom
	M.adjust_position=function(it,dx,dy,dz)
		local vs=it.verts
		for i=1,#vs do local v=vs[i]
			v[1]=v[1]+dx
			v[2]=v[2]+dy
			v[3]=v[3]+dz
		end
		return it
	end

	-- build lines from polys
	M.build_lines=function(it)

		local ld={}
		local addline
		addline=function(l1,l2)
			if l1>l2 then return addline(l2,l1) end -- force low -> high
			if not ld[l1] then ld[l1]={} end
			ld[l1][l2]=true -- merge all lines
		end
		
		for i,p in ipairs(it.polys) do -- each poly
			local hp=#p
			for ip,vp in ipairs(p) do -- each line
				addline(vp,p[(ip%hp)+1])
			end
		end

		local lines={} -- pull the lines out of the merged tables
		it.lines=lines
		for l1,v in pairs(ld) do
			for l2,b in pairs(v) do
				lines[#lines+1]={l1,l2}
			end
		end
		return it
	end

	-- build the edges
	M.build_edges=function(it)
		local es={}
		it.edges_map=es
		for ip,vp in ipairs(it.polys) do
			local hvp=#vp
			for i1=1,hvp do
				local i2=i1+1
				if i2>hvp then i2=1 end -- wrap 
				local v1=vp[i1]
				local v2=vp[i2]
				if v2>v1 then v1,v2=v2,v1 end -- sort order
				es[v1]=es[v1] or {}
				es[v1][v2]=es[v1][v2] or {}
				es[v1][v2][ip]=i1 -- what polys it links too and by what vertexs
			end
		end
		
		it.edges={}
		for i1,v in pairs(es) do
			for i2,b in pairs(v) do
				it.edges[#it.edges+1]={i1,i2} -- each edge will only get added once
			end
		end
		return it
	end
	
-- uses first 3 verts, does not fix the length
	M.get_poly_normal=function(it,p)
	
		local v1=it.verts[ p[1] ]
		local v2=it.verts[ p[2] ]
		local v3=it.verts[ p[3] ]
--		if not v1 or not v2 or not v3 then return {0,0,0} end
		local va={}
		local vb={}
		va[1]=v1[1]-v2[1]
		va[2]=v1[2]-v2[2]
		va[3]=v1[3]-v2[3]
		vb[1]=v3[1]-v2[1]
		vb[2]=v3[2]-v2[2]
		vb[3]=v3[3]-v2[3]
		
		-- face normal
		local vn={}
		vn[1]=va[3]*vb[2] - va[2]*vb[3]
		vn[2]=va[1]*vb[3] - va[3]*vb[1]
		vn[3]=va[2]*vb[1] - va[1]*vb[2]
		
		return vn
	end

	M.normalize=function(v)
		local dd=v[1]*v[1] + v[2]*v[2] + v[3]*v[3]
		local d=math.sqrt(dd)
		if d<(1/65536) then d=1 end
		v[1]=v[1]/d
		v[2]=v[2]/d
		v[3]=v[3]/d
		return v
	end


	M.face_square_uvs=function(it)
		local t={
			{0,0},
			{1,0},
			{1,1},
			{0,1},
		}
		for i,v in pairs(it.verts) do
			local idx=((v[8]-1)%4)+1
			v[7]=t[idx][1]
			v[8]=t[idx][2]
		end
		return it
	end

	M.flip=function(it)
		for i,p in pairs(it.polys) do M.poly_flip(it,p) end
		for i,v in pairs(it.verts) do M.vert_flip(it,v) end
		return it
	end
		
	M.vert_flip=function(it,v)
		if v[4] then
			v[4]=-v[4]
			v[5]=-v[5]
			v[6]=-v[6]
		end
		return it
	end

	M.poly_flip=function(it,p)
	
		local n={}
		for i=#p,1,-1 do
			n[#n+1]=p[i]
		end
		for i=1,#n do
			p[i]=n[i]
		end

		return it
	end


	M.flipaxis=function(it,axis)
		local mv=#it.verts
		for iv=1,mv do local vv=it.verts[iv]
			vv[axis]=-vv[axis]
			vv[axis+3]=-vv[axis+3]
			vv[axis+8]=-vv[axis+8]
		end
	end
	
-- dupe+mirror point and polygons around the origin on the given axis 1=x 2=y 3=z
	M.mirror=function(it,axis,cb)
	
		local vmap={} -- map vertexes to their mirrored one
		
		local dupe=function(it)
			local r={}
			for i,v in ipairs(it) do r[i]=v end
			r.mat=it.mat
			return r
		end

		local mv=#it.verts
		for iv=1,mv do local vv=it.verts[iv]
			if vv[axis] == 0 then -- noflip
				if cb then cb(vv,false) end
			else
				local id=#it.verts+1
				local vd=dupe(vv)
				it.verts[id]=vd
				vmap[iv]=id
				vd[axis]=-vd[axis]
				vd[axis+3]=-vd[axis+3]
				if cb then cb(vd,true) end
			end
		end

		local mp=#it.polys
		for ip=1,mp do local vp=it.polys[ip]
				local id=#it.polys+1
				local vd=dupe(vp)
				it.polys[id]=vd
				for i,v in ipairs(vd) do
					vd[i]=vmap[v] or v
				end
				M.poly_flip(it,vd)
		end
	
	end

-- unmerge the vertexs so they are unique per face and build normals so we get flat shading
	M.build_flat_normals=function(it)
	
		local verts={}
		local polys={}

		local iv=0
		local ip=0
		for _,vp in ipairs(it.polys) do
			ip=ip+1
			polys[ip]={mat=vp.mat}
			for i=1,#vp do
				local v=it.verts[ vp[i] ]
				iv=iv+1
				verts[iv]={unpack(v)}
				polys[ip][i]=iv
			end
		end
		
		it.verts=verts
		it.polys=polys
		
		M.build_normals(it)

		return it
	end


	M.build_normals=function(it)

-- reset normals
		for iv,vv in ipairs(it.verts) do
			vv[4]=0
			vv[5]=0
			vv[6]=0
			vv.count=0
		end
		
-- add each face to normals
		for ip,vp in ipairs(it.polys) do
			local n=M.normalize( M.get_poly_normal(it,vp) )
			for i=1,#vp do
				local v=it.verts[ vp[i] ]
				v[4]=v[4]+n[1]
				v[5]=v[5]+n[2]
				v[6]=v[6]+n[3]
				v.count=v.count+1
			end
		end

-- normalize the normals and remove count
		for iv,vv in ipairs(it.verts) do
			if vv.count>0 then
				vv[4]=vv[4]/vv.count
				vv[5]=vv[5]/vv.count
				vv[6]=vv[6]/vv.count
			end
			vv.count=nil
			
			if vv[4]~=vv[4] then vv[4]=1 end	-- if nan
			if vv[5]~=vv[5] then vv[5]=1 end
			if vv[6]~=vv[6] then vv[6]=1 end
			
			if vv[4]==0 and vv[5]==0 and vv[6]==0 then -- if zero length
				vv[4]=1
				vv[5]=1
				vv[6]=1
			end
		end

		return it
	end


	M.merge_points_by_pos=function(it,snap)

		local hash={} -- map string to
		local map={} -- map old points to new points
		local verts={}
		local idx=1
		for iv,vv in ipairs(it.verts) do
			if snap then
				vv[1]=math.floor(0.5+(vv[1]/snap))*snap
				vv[2]=math.floor(0.5+(vv[2]/snap))*snap
				vv[3]=math.floor(0.5+(vv[3]/snap))*snap
			end
			local s=vv[1]..","..vv[2]..","..vv[3]
			if not hash[s] then
				hash[s]=idx
				verts[idx]=vv
				idx=idx+1
			end
			map[iv]=hash[s]
		end
		it.verts=verts -- new verts
		for ip,vp in ipairs(it.polys) do
			for i=1,#vp do
				vp[i] = map[ vp[i] ] -- change vert indexs
			end
		end
	end

	M.build_tangents=function(it)

-- reset tangents
		for iv,vv in ipairs(it.verts) do
			vv[9]=0
			vv[10]=0
			vv[11]=0
			vv[12]={0,0,0}
		end
		
-- check each poly edge and add a weighted version of its normal to the tangent
		for ip,vp in ipairs(it.polys) do
			for i=1,#vp do
				local v1=it.verts[ vp[i] ]
				local v2=it.verts[ vp[(i%#vp)+1] ]
				local n={ v2[1]-v1[1] , v2[2]-v1[2] , v2[3]-v1[3] , v2[7]-v1[7] , v2[8]-v1[8] }

				local dd=math.sqrt(n[1]*n[1] + n[2]*n[2] + n[3]*n[3]) -- unit length
				if dd==0 then dd=1 end
				n[1]=n[1]/dd
				n[2]=n[2]/dd
				n[3]=n[3]/dd
				local dd=math.sqrt(n[4]*n[4] + n[5]*n[5] ) -- unit length
				if dd==0 then dd=1 end
				n[4]=n[4]/dd
				n[5]=n[5]/dd

				v1[ 9]=v1[ 9]+(n[1]*n[5])
				v1[10]=v1[10]+(n[2]*n[5])
				v1[11]=v1[11]+(n[3]*n[5])

				v1[12][1]=v1[12][1]+(n[1]*n[4])
				v1[12][2]=v1[12][2]+(n[2]*n[4])
				v1[12][3]=v1[12][3]+(n[3]*n[4])

				v2[ 9]=v2[ 9]+(n[1]*n[5])
				v2[10]=v2[10]+(n[2]*n[5])
				v2[11]=v2[11]+(n[3]*n[5])

				v2[12][1]=v2[12][1]+(n[1]*n[4])
				v2[12][2]=v2[12][2]+(n[2]*n[4])
				v2[12][3]=v2[12][3]+(n[3]*n[4])

			end
		end
		
-- merge tangents of vertexs that are in same 3d location
--[[
		local merge={}
		for iv,vv in ipairs(it.verts) do
			local s=vv[1]..","..vv[2]..","..vv[3]
			merge[s]=merge[s] or {}
			local t=merge[s]
			t[#t+1]=vv
		end
		for nv,vv in pairs(merge) do
			if #vv>1 then
				for i=2,#vv do
					vv[1][ 9]=vv[1][ 9]+vv[i][ 9]
					vv[1][10]=vv[1][10]+vv[i][10]
					vv[1][11]=vv[1][11]+vv[i][11]
					vv[1][12][1]=vv[1][12][1]+vv[i][12][1]
					vv[1][12][2]=vv[1][12][2]+vv[i][12][2]
					vv[1][12][3]=vv[1][12][3]+vv[i][12][3]
					vv[i][ 9]=vv[1][ 9]
					vv[i][10]=vv[1][10]
					vv[i][11]=vv[1][11]
					vv[i][12][1]=vv[1][12][1]
					vv[i][12][2]=vv[1][12][2]
					vv[i][12][3]=vv[1][12][3]
				end
			end
		end
]]

-- remove normal to place tangent on surface and then fix its length
		for iv,vv in ipairs(it.verts) do
			local n=( (vv[9]*vv[4]) + (vv[10]*vv[5]) + (vv[11]*vv[6]) ) -- dot
			vv[ 9]=vv[ 9]-(n*vv[4])
			vv[10]=vv[10]-(n*vv[5])
			vv[11]=vv[11]-(n*vv[6])

			local n=( (vv[12][1]*vv[4]) + (vv[12][2]*vv[5]) + (vv[12][3]*vv[6]) ) -- dot
			vv[12][1]=vv[12][1]-(n*vv[4])
			vv[12][2]=vv[12][2]-(n*vv[5])
			vv[12][3]=vv[12][3]-(n*vv[6])

			local dd=math.sqrt(vv[9]*vv[9] + vv[10]*vv[10] + vv[11]*vv[11])
			if dd==0 then dd=1 end
			vv[ 9]=vv[ 9]/dd
			vv[10]=vv[10]/dd
			vv[11]=vv[11]/dd

			local dd=math.sqrt(vv[12][1]*vv[12][1] + vv[12][2]*vv[12][2] + vv[12][3]*vv[12][3])
			if dd==0 then dd=1 end
			vv[12][1]=vv[12][1]/dd
			vv[12][2]=vv[12][2]/dd
			vv[12][3]=vv[12][3]/dd
		end

-- force tangent and bitangent to be at right angles of each other

		for iv,vv in ipairs(it.verts) do
		
			local n={vv[4],vv[5],vv[6]}
			local t={vv[9],vv[10],vv[11]}
			local b={vv[12][1],vv[12][2],vv[12][3]}
			
			local h=tardis.v3.new(t):add(b):normalize() -- halfway vector
			
			local c=tardis.v3.new(h):cross(n):normalize()
			
			local t2=tardis.v3.new(h):add(c):normalize() -- new vector at right angles
			local b2=tardis.v3.new(h):sub(c):normalize() -- new vector at right angles
			
			if tardis.v3.dot(t,t2) > tardis.v3.dot(t,b2) then -- right way around
			
				vv[9]=t2[1]
				vv[10]=t2[2]
				vv[11]=t2[3]

				vv[12][1]=b2[1]
				vv[12][2]=b2[2]
				vv[12][3]=b2[3]
			
			else
			
				vv[9]=b2[1]
				vv[10]=b2[2]
				vv[11]=b2[3]

				vv[12][1]=t2[1]
				vv[12][2]=t2[2]
				vv[12][3]=t2[3]

			end

		end


-- work out the sign for the bitangent and force remove any nan values
		for iv,vv in ipairs(it.verts) do
			local b={ (vv[5]*vv[11])-(vv[6]*vv[10]) , (vv[6]*vv[9])-(vv[4]*vv[11]) , (vv[4]*vv[10])-(vv[5]*vv[9]) }
			vv[12] = vv[12][1]*b[1] + vv[12][2]*b[2] + vv[12][3]*b[3] 
			if vv[12]>=0 then vv[12]=1 else vv[12]=-1 end
			if vv[ 9]~=vv[ 9] then vv[ 9]=1 end
			if vv[10]~=vv[10] then vv[10]=1 end
			if vv[11]~=vv[11] then vv[11]=1 end
		end

		return it
	end

-- face polys away from the origin
	M.fix_poly_order=function(it,p)
	
		local v2=it.verts[ p[2] ]
		local vn=M.get_poly_normal(it,p)
		
		local d = vn[1]*v2[1] + vn[2]*v2[2] + vn[3]*v2[3]
		
		if d<0 then -- invert poly order
			M.poly_flip(it,p)
		end

		return it
	end

	-- bevil a base object, s is how much each face is scaled, so 7/8 is a nice bevel
	M.apply_bevel=function(it,s)
	
		M.build_edges(it)

		local vs={}
		local function vcopy(i)
			local n={} for a,b in ipairs(it.verts[i]) do n[a]=b end
			vs[#vs+1]=n -- new vertex
			return #vs -- new vertex id
		end
		for ip,vp in ipairs(it.polys) do
			local n=M.normalize( M.get_poly_normal(it,vp) )
			for i=1,#vp do
				vp[i]=vcopy(vp[i])
				local v=vs[ vp[i] ]
				v[4]=n[1]
				v[5]=n[2]
				v[6]=n[3]
				v[7]=ip
				v[8]=i
			end

		end
		it.verts=vs

		for ip,vp in ipairs(it.polys) do
			local vc={0,0,0} -- calculate center of face
			local ic=0
			for i,v in ipairs(vp) do local vv=it.verts[v]
				vc[1]=vc[1]+vv[1]
				vc[2]=vc[2]+vv[2]
				vc[3]=vc[3]+vv[3]
				ic=ic+1
			end
			vc[1]=vc[1]/ic
			vc[2]=vc[2]/ic
			vc[3]=vc[3]/ic
			for i,v in ipairs(vp) do local vv=it.verts[v]
				vv[1]=vc[1] + ((vv[1]-vc[1])*s) -- scale face around center
				vv[2]=vc[2] + ((vv[2]-vc[2])*s) -- each vertex will be unique to each face
				vv[3]=vc[3] + ((vv[3]-vc[3])*s)
			end
		end

		-- add in poly where the old edges used to be
		local js={}
		local es=it.edges_map
		for i,e in pairs(it.edges) do
			local p1,v1
			local p2,v2
			for ip,iv in pairs( es[ e[1] ][ e[2] ]) do
				if p1 then p2=ip v2=iv else p1=ip v1=iv end
			end
			p1=it.polys[p1] -- lookup polys, will have new vertex ids
			p2=it.polys[p2] -- build new join poly using these new vertex ids
			local p={}
			p[1] = p1[v1]
			p[3] = p2[v2]
			v1=v1+1 if v1>#p1 then v1=1 end
			v2=v2+1 if v2>#p2 then v2=1 end
			p[2] = p1[v1]
			p[4] = p2[v2]
			js[#js+1]={p[1],p[4]}
			js[#js+1]={p[2],p[3]}

			M.fix_poly_order(it,p)
			it.polys[#it.polys+1]=p -- add join poly
		end
		

		local function dojoin(v2,v1) -- insert v1 into v1
			if v1[1] == v2[1] then
				for i=2,#v1 do
					table.insert(v2,1,v1[i])
				end
			elseif v1[#v1] == v2[1] then
				for i=#v1-1,1,-1 do
					table.insert(v2,1,v1[i])
				end
			elseif v1[1] == v2[#v2] then
				for i=2,#v1 do
					table.insert(v2,v1[i])
				end
			elseif v1[#v1] == v2[#v2] then
				for i=#v1-1,1,-1 do
					table.insert(v2,v1[i])
				end
			end

		end
		
		for i1,v1 in pairs(js) do
--			if #v1 == 2 then -- unjoined
				for i2,v2 in pairs(js) do
					if v1~=v2 then -- do not join to self
						if v2[#v2] == v1[1] then
							dojoin(v2,v1)
							js[i1]=nil -- joined
							break
						elseif v2[#v2] == v1[#v1] then
							dojoin(v2,v1)
							js[i1]=nil -- joined
							break
						elseif v2[1] == v1[1] then
							dojoin(v2,v1)
							js[i1]=nil -- joined
							break
						elseif v2[1] == v1[#v1] then
							dojoin(v2,v1)
							js[i1]=nil -- joined
							break
						end
					end
				end
--			end
		end
		
		for i,v in pairs(js) do -- add pollys
			v[#v]=nil
			it.polys[#it.polys+1]=v
			M.fix_poly_order(it,v)
		end
			
		for i,p in pairs(it.polys) do
			M.fix_poly_order(it,p)
		end
			
		return it
	end

-- find the radius of this object from 0,0,0

	M.max_radius=function(it)

		local maxdd=0
		
		for iv,vv in ipairs(it.verts) do
			local dd=vv[1]*vv[1]+vv[2]*vv[2]+vv[3]*vv[3]
			if dd>maxdd then maxdd=dd end
		end

		return math.sqrt(maxdd)
	end

	M.find_bounds=function(it)

		local dd={0,0,0,0,0,0}
		
		local v=it.verts[1]
		if v then

			dd[1]=v[1]
			dd[2]=v[1]
			dd[3]=v[2]
			dd[4]=v[2]
			dd[5]=v[3]
			dd[6]=v[3]
		
			for iv,vv in ipairs(it.verts) do
				if vv[1]<dd[1] then dd[1]=vv[1] end
				if vv[1]>dd[2] then dd[2]=vv[1] end
				if vv[2]<dd[3] then dd[3]=vv[2] end
				if vv[2]>dd[4] then dd[4]=vv[2] end
				if vv[3]<dd[5] then dd[5]=vv[3] end
				if vv[3]>dd[6] then dd[6]=vv[3] end
			end

		end
		
		return dd
	end

	M.find_center=function(it)
		local dd=M.find_bounds(it)
		return (dd[1]+dd[2])/2 , (dd[3]+dd[4])/2 , (dd[5]+dd[6])/2
	end

	-- get collision triangle,vertex collision tables
	-- optional m4 transform and material allow/block filtering table maps
	M.get_colision_tables=function(it,m4,allows,blocks)

		local allow
		local block
		if allows then allow={} for i,v in ipairs(allows) do allow[v]=true end end
		if blocks then block={} for i,v in ipairs(blocks) do block[v]=true end end

		local need={}
		local map={}

		local tp={}
		local tv={}
		for i,p in ipairs(it.polys) do -- scan and filter polys
			local ok=true
			if allow and ( not allow[ p.mat or 0 ] ) then ok=false end
			if block and (     block[ p.mat or 0 ] ) then ok=false end
			if ok then
				for ti=0,(#p-3) do
					for i=0,2,1 do
						local tv=1+ti+i if i==0 then tv=1 end
						tp[#tp+1]=p[tv]-1
						need[ p[tv]-1 ]=true
					end
				end
			end
		end
		local idx=0 -- actual verts used
		for i,v in ipairs(it.verts) do
			map[i-1]=idx
			if need[ i-1 ] then
				local l=#tv
				local v=V4{v[1],v[2],v[3],1}
				if m4 then v:product(m4) end -- adjust position
				tv[l+1]=v[1]
				tv[l+2]=v[2]
				tv[l+3]=v[3]
				idx=idx+1
			end
		end

		for i=1,#tp do -- remap
			tp[i] = map[ tp[i] ]
		end

		return tp,tv

	end

-- build a center vertex between all the given vertices
	M.make_center_vertex=function(it,...)

		local p={...}
		local c={}

		local vm=0 -- find max (sanity)
		for iv=1,#p do
			local v=it.verts[ p[iv] ]
			vm=math.max(vm,#v)
		end
		for iv=1,#p do
			local v=it.verts[ p[iv] ]
			for i=1,vm do
			c[i]=(c[i] or 0)+(v[i] or 0)
			end
		end
		for i=1,vm do -- average
			c[i]=c[i]/#p
		end

		return c
	end


-- only subdivide polys *within* the given radius from the origin
	M.subdivide_radius=function(it,radius)

		local ps={}
		local np=#it.polys
		for ip=1,np do
			local p=it.polys[ip]

			local mask=0
			local vb=#it.verts
			for i=1,#p do
				local ib=i-1
				if ib==0 then ib=#p end -- wrap
				local c=M.make_center_vertex(it,p[ib],p[i])
				local dd=c[1]*c[1]+c[2]*c[2]+c[3]*c[3]
				local d=math.sqrt(dd)
				if d<=radius then
					mask=mask+math.pow(2,i-1)
					it.verts[ #it.verts+1 ]=c
				end
			end

			local vl=#it.verts
			if #p==3 then
				if mask==1+2+4 then -- all 3 sides are split
					ps[#ps+1]={ p[1] , vl-3+2 , vl-3+1 , mat=p.mat }
					ps[#ps+1]={ p[2] , vl-3+3 , vl-3+2 , mat=p.mat }
					ps[#ps+1]={ p[3] , vl-3+1 , vl-3+3 , mat=p.mat }
					ps[#ps+1]={ vl-2 , vl-1   , vl-0   , mat=p.mat }
				elseif mask==1 then -- single 3-1 split
					ps[#ps+1]={ p[3] , vl     , p[2]   , mat=p.mat }
					ps[#ps+1]={ vl   , p[1]   , p[2]   , mat=p.mat }
				elseif mask==2 then -- single 1-2 split
					ps[#ps+1]={ p[1] , vl     , p[3]   , mat=p.mat }
					ps[#ps+1]={ vl   , p[2]   , p[3]   , mat=p.mat }
				elseif mask==4 then -- single 2-3 split
					ps[#ps+1]={ p[2] , vl     , p[1]   , mat=p.mat }
					ps[#ps+1]={ vl   , p[3]   , p[1]   , mat=p.mat }
				elseif mask==1+2 then -- double 3-1 1-2 split
					ps[#ps+1]={ vl-1 , p[1]   , vl     , mat=p.mat }
					ps[#ps+1]={ p[3] , vl-1   , vl     , mat=p.mat }
					ps[#ps+1]={ p[3] , vl     , p[2]   , mat=p.mat }
				elseif mask==1+4 then -- double 3-1 2-3 split
					ps[#ps+1]={ vl-1 , p[1]   , p[2]   , mat=p.mat }
					ps[#ps+1]={ p[2] , vl     , vl-1   , mat=p.mat }
					ps[#ps+1]={ vl   , p[3]   , vl-1   , mat=p.mat }
				elseif mask==2+4 then -- double 1-2 2-3 split
					ps[#ps+1]={ p[1] , vl-1   , vl     , mat=p.mat }
					ps[#ps+1]={ vl-1 , p[2]   , vl     , mat=p.mat }
					ps[#ps+1]={ vl   , p[3]   , p[1]   , mat=p.mat }
				elseif mask==0 then -- no split
					ps[#ps+1]={ p[1] , p[2] , p[3] , mat=p.mat }
				end
			else
				ps[#ps+1]=p
			end

			
		end

		it.polys=ps -- new polygons

		return it
	end

-- subdivide polys
	M.subdivide=function(it,radius)

		if radius then return M.subdivide_radius(it,radius) end

		local ps={}
		local np=#it.polys
		for ip=1,np do
			local p=it.polys[ip]

			for i=1,#p do
				local ib=i-1
				if ib==0 then ib=#p end -- wrap
				it.verts[ #it.verts+1 ]=M.make_center_vertex(it,p[ib],p[i])
			end

			if #p==3 then 
				ps[#ps+1]={p[1],#it.verts-3+2,#it.verts-3+1,mat=p.mat}
				ps[#ps+1]={p[2],#it.verts-3+3,#it.verts-3+2,mat=p.mat}
				ps[#ps+1]={p[3],#it.verts-3+1,#it.verts-3+3,mat=p.mat}
				ps[#ps+1]={#it.verts-2,#it.verts-1,#it.verts-0,mat=p.mat}
			else
				ps[#ps+1]=p
			end

			
		end

		it.polys=ps -- new polygons

		return it
	end

	require("wetgenes.gamecake.spew.geom_mask").fill(M)
	require("wetgenes.gamecake.spew.geom_solids").fill(M)


M.bake=function(oven,geom)
	geom=geom or {}
	for n,v in pairs(M) do geom[n]=v end
	geom.modname=M.modname

-- we have OpenGL access here
	geom.meta={__index=geom}
	geom.new=function(it) it=it or {} setmetatable(it,geom.meta) return it:reset() end

	
	require("wetgenes.gamecake.spew.geom_draw").fill(oven,geom)

	return geom
end


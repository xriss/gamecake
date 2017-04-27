--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

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

M.bake=function(oven,geom)
	geom=geom or {}
	geom.modname=M.modname

	geom.meta={__index=geom}
	geom.new=function(it) it=it or {} setmetatable(it,geom.meta) return it end

	geom.duplicate=function(it)
		return geom.copy(it,geom.new())
	end

	geom.copy=function(src,dst)
-- reset
		dst.verts={}
		dst.mats={}
		dst.polys={}
		dst.name=src.name
		dst:clear_predraw()
-- copy mats
		for im = 1 , #src.mats do local mat=src.mats[im]
			local m={}
			m.name=mat.name
			m.diffuse={unpack(mat.diffuse)}
			m.specular={unpack(mat.specular)}
			m.shininess={unpack(mat.shininess)}
			dst.mats[ im ]=m
		end
-- copy verts
		for iv = 1 , #src.verts do local vert=src.verts[iv]
			localv={unpack(vert)}
			dst.verts[ iv ]=v
		end
--copy polys		
		for iv = 1 , #src.polys do local poly=src.polys[iv]
			local p={unpack(poly)}
			p.mat=mats_map_idx[poly.mat]
			dst.polys[ iv ]=p
		end
		return dst
	end


geom.idxs=	{
				POS_X= 1, POS_Y= 2, POS_Z= 3,
				NRM_X= 4, NRM_Y= 5, NRM_Z= 6,
				TEX_U= 7, TEX_V= 8,
				TAN_X= 9, TAN_Y=10, TAN_Z=11, TAN_W=12,
				BNE_A=13, BNE_B=14, BNE_C=15, BNE_D=16
			}


	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	
	require("wetgenes.gamecake.spew.geom_draw").fill(oven,geom)
	require("wetgenes.gamecake.spew.geom_solids").fill(oven,geom)
	require("wetgenes.gamecake.spew.geom_mask").fill(oven,geom)

		
	-- scale the geom
	geom.adjust_scale=function(it,s)
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
	geom.adjust_position=function(it,dx,dy,dz)
		local vs=it.verts
		for i=1,#vs do local v=vs[i]
			v[1]=v[1]+dx
			v[2]=v[2]+dy
			v[3]=v[3]+dz
		end
		return it
	end

	-- build lines from polys
	geom.build_lines=function(it)

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
	geom.build_edges=function(it)
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
	geom.get_poly_normal=function(it,p)
	
		local v1=it.verts[ p[1] ]
		local v2=it.verts[ p[2] ]
		local v3=it.verts[ p[3] ]
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

	geom.normalize=function(v)
		local dd=v[1]*v[1] + v[2]*v[2] + v[3]*v[3]
		local d=math.sqrt(dd)
		if d<(1/65536) then d=1 end
		v[1]=v[1]/d
		v[2]=v[2]/d
		v[3]=v[3]/d
		return v
	end


	geom.face_square_uvs=function(it)
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

	geom.flip=function(it)
		for i,p in pairs(it.polys) do geom.poly_flip(it,p) end
		for i,v in pairs(it.verts) do geom.vert_flip(it,v) end
		return it
	end
		
	geom.vert_flip=function(it,v)
		if v[4] then
			v[4]=-v[4]
			v[5]=-v[5]
			v[6]=-v[6]
		end
		return it
	end

	geom.poly_flip=function(it,p)
	
		local n={}
		for i=#p,1,-1 do
			n[#n+1]=p[i]
		end
		for i=1,#n do
			p[i]=n[i]
		end

		return it
	end


	geom.flipaxis=function(it,axis)
		local mv=#it.verts
		for iv=1,mv do local vv=it.verts[iv]
			vv[axis]=-vv[axis]
			vv[axis+3]=-vv[axis+3]
			vv[axis+8]=-vv[axis+8]
		end
	end
	
-- dupe+mirror point and polygons around the origin on the given axis 1=x 2=y 3=z
	geom.mirror=function(it,axis,cb)
	
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
				geom.poly_flip(it,vd)
		end
	
	end

	geom.build_normals=function(it)

-- reset normals
		for iv,vv in ipairs(it.verts) do
			vv[4]=0
			vv[5]=0
			vv[6]=0
			vv.count=0
		end
		
-- add each face to normals
		for ip,vp in ipairs(it.polys) do
			local n=geom.normalize( geom.get_poly_normal(it,vp) )
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
		end

		return it
	end

	geom.build_tangents=function(it)

-- reset tangents
		for iv,vv in ipairs(it.verts) do
			vv[9]=0
			vv[10]=0
			vv[11]=0
			vv[12]=0
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

				v2[ 9]=v2[ 9]+(n[1]*n[5])
				v2[10]=v2[10]+(n[2]*n[5])
				v2[11]=v2[11]+(n[3]*n[5])


			end
		end
		
-- merge tangents of vertexs that are in same 3d location
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
					vv[i][ 9]=vv[1][ 9]
					vv[i][10]=vv[1][10]
					vv[i][11]=vv[1][11]
				end
			end
		end

-- remove normal to place tangent on surface and then fix its length
		for iv,vv in ipairs(it.verts) do
			local n=( (vv[9]*vv[4]) + (vv[10]*vv[5]) + (vv[11]*vv[6]) ) -- dot
			vv[ 9]=vv[ 9]-(n*vv[4])
			vv[10]=vv[10]-(n*vv[5])
			vv[11]=vv[11]-(n*vv[6])

			local dd=math.sqrt(vv[9]*vv[9] + vv[10]*vv[10] + vv[11]*vv[11])
			if dd==0 then dd=1 end
			vv[ 9]=vv[ 9]/dd
			vv[10]=vv[10]/dd
			vv[11]=vv[11]/dd
		end

-- work out the sign for the bitangent
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

				n[1]=(n[1]*n[4])
				n[2]=(n[2]*n[4])
				n[3]=(n[3]*n[4])

				local b={ -- bitangent is cross product of normal and tangent
							(v1[2]*v1[11])-(v1[3]*v1[10]) ,
							(v1[3]*v1[9] )-(v1[1]*v1[11]) ,
							(v1[1]*v1[10])-(v1[2]*v1[9] )
						}

				v1[12]=v1[12]+( (b[1]*n[1]) + (b[2]*n[2]) + (b[3]*n[3]) )
			end
		end
		for iv,vv in ipairs(it.verts) do
			if vv[12]>=0 then vv[12]=1 else vv[12]=-1 end
		end

		return it
	end

-- face polys away from the origin
	geom.fix_poly_order=function(it,p)
	
		local v2=it.verts[ p[2] ]
		local vn=geom.get_poly_normal(it,p)
		
		local d = vn[1]*v2[1] + vn[2]*v2[2] + vn[3]*v2[3]
		
		if d>0 then -- invert poly order
			geom.poly_flip(it,p)
		end

		return it
	end

	-- bevil a base object, s is how much each face is scaled, so 7/8 is a nice bevel
	geom.apply_bevel=function(it,s)
	
		geom.build_edges(it)

		local vs={}
		local function vcopy(i)
			local n={} for a,b in ipairs(it.verts[i]) do n[a]=b end
			vs[#vs+1]=n -- new vertex
			return #vs -- new vertex id
		end
		for ip,vp in ipairs(it.polys) do
			local n=geom.normalize( geom.get_poly_normal(it,vp) )
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

			geom.fix_poly_order(it,p)
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
			geom.fix_poly_order(it,v)
		end
			
		for i,p in pairs(it.polys) do
			geom.fix_poly_order(it,p)
		end
			
		return it
	end

-- find the radius of this object from 0,0,0

	geom.max_radius=function(it)

		local maxdd=0
		
		for iv,vv in ipairs(it.verts) do
			local dd=vv[1]*vv[1]+vv[2]*vv[2]+vv[3]*vv[3]
			if dd>maxdd then maxdd=dd end
		end

		return math.sqrt(maxdd)
	end

	return geom
end


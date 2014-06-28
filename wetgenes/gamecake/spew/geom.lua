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

M.bake=function(oven,geom)
	geom=geom or {}
	geom.modname=M.modname

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	
		
	-- scale the geom
	geom.adjust_scale=function(it,s)
		s=s or 1
		local vs=it.verts
		for i=1,#vs do local v=vs[i]
			v[1]=v[1]*s
			v[2]=v[2]*s
			v[3]=v[3]*s
		end
	end

	-- allocate and upload the geom to opengl buffers
	geom.predraw=function(it,progname)

		if not it.predrawn then
			local orders={}
			orders[3]={ {3,3,2,1,1}			,	{1,1,2,3,3}		}
			orders[4]={ {3,3,4,2,1,1}		,	{1,1,2,4,3,3}	}
			orders[5]={ {4,4,3,5,2,1,1}		,	{1,1,2,5,3,4,4}	}

			local t={}
			local f=1
			for i,p in ipairs(it.polys) do
	--			local c=cs[1+(i%#cs)]
				local o=orders[#p][1+f%2]
				for _,i in ipairs(o) do
					local idx=p[i]
					local v=it.verts[idx]
					t[#t+1]=v[1]
					t[#t+1]=v[2]
					t[#t+1]=v[3]

					t[#t+1]=v[4] or 0
					t[#t+1]=v[5] or 0
					t[#t+1]=v[6] or 0

					t[#t+1]=v[7] or 0
					t[#t+1]=v[8] or 0
					t[#t+1]=(p.mat or 1)-1

					f=f+1
				end
			end
			it.predrawn=flat.array_predraw({fmt="xyznrmuvm",data=t,progname=progname,array=gl.TRIANGLE_STRIP,vb=true})
		end
		
	end

	-- draw the geom (upload first, mkay)
	geom.draw=function(it,progname,cb)
		geom.predraw(it,progname)
		it.predrawn.draw(cb)
	end	
	
	-- allocate and upload the geom to opengl buffers
	geom.predraw_lines=function(it,progname)

		if not it.predrawn_lines then

			local t={}
			local f=1
			for i,p in ipairs(it.lines) do
				for i=1,2 do
					local idx=p[i]
					local v=it.verts[idx]
					t[#t+1]=v[1]
					t[#t+1]=v[2]
					t[#t+1]=v[3]

					t[#t+1]=v[4] or 0
					t[#t+1]=v[5] or 0
					t[#t+1]=v[6] or 0

					t[#t+1]=v[7] or 0
					t[#t+1]=v[8] or 0
					t[#t+1]=(p.mat or 1)-1

					f=f+1
				end
			end
			it.predrawn_lines=flat.array_predraw({fmt="xyznrmuvm",data=t,progname=progname,array=gl.LINES,vb=true})
		end
		
	end

	-- draw the geom (upload first, mkay)
	geom.draw_lines=function(it,progname,cb)
		geom.predraw_lines(it,progname)
		it.predrawn_lines.draw(cb)
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
	end

	geom.flip=function(it)

		for i,p in pairs(it.polys) do geom.poly_flip(it,p) end
		for i,v in pairs(it.verts) do geom.vert_flip(it,v) end
	end
		
	geom.vert_flip=function(it,v)
		if v[4] then
			v[4]=-v[4]
			v[5]=-v[5]
			v[6]=-v[6]
		end
	end

	geom.poly_flip=function(it,p)
	
		local n={}
		for i=#p,1,-1 do
			n[#n+1]=p[i]
		end
		for i=1,#n do
			p[i]=n[i]
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

	end

	geom.fix_poly_order=function(it,p)
	
		local v2=it.verts[ p[2] ]
		local vn=geom.get_poly_normal(it,p)
		
		local d = vn[1]*v2[1] + vn[2]*v2[2] + vn[3]*v2[3]
		
		if d>0 then -- invert poly order
			geom.poly_flip(it,p)
		end
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
			
	end


	geom.tetrahedron=function(it)
		it=it or {}
		
		it.verts={
				{ 0.5, 0.5, 0.5},
				{-0.5, 0.5,-0.5},
				{ 0.5,-0.5,-0.5},
				{-0.5,-0.5, 0.5},
			}
			
		it.polys={
				{1,2,3},
				{2,4,3},
				{1,3,4},
				{1,4,2},
			}
			
--		it.strips={1,2,3,4,1,2}
		
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

	geom.octahedron=function(it)
		it=it or {}
		
		local a=1/(2*math.sqrt(2))
		local b=1/2
		
		it.verts={
				{ a, 0, a},
				{ a, 0,-a},
				{-a, 0, a},
				{-a, 0,-a},
				{ 0, b, 0},
				{ 0,-b, 0},
			}
			
		it.polys={
				{3,4,5},
				{4,2,5},
				{2,1,5},
				{1,3,5},
				{2,4,6},
				{4,3,6},
				{1,2,6},
				{3,1,6},
			}
			
--		it.strips={}
		
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end
	
	geom.hexahedron=function(it)
		it=it or {}
		
		it.verts={
				{ 0.5, 0.5, 0.5},
				{ 0.5, 0.5,-0.5},
				{ 0.5,-0.5, 0.5},
				{ 0.5,-0.5,-0.5},
				{-0.5, 0.5, 0.5},
				{-0.5, 0.5,-0.5},
				{-0.5,-0.5, 0.5},
				{-0.5,-0.5,-0.5},
			}
			
		it.polys={
				{8,4,3,7},
				{8,7,5,6},
				{7,3,1,5},
				{6,5,1,2},
				{4,2,1,3},
				{8,6,2,4},
			}
			
--		it.strips={}
		
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

	geom.icosahedron=function(it)
		it=it or {}
		
		local a=1/2
		local b=1/(1+math.sqrt(5))
		
		it.verts={
				{ 0, b, a}, -- 1
				{ 0, b,-a}, -- 2
				{ 0,-b, a}, -- 3
				{ 0,-b,-a}, -- 4
				{ a, 0, b}, -- 5
				{ a, 0,-b}, -- 6
				{-a, 0, b}, -- 7
				{-a, 0,-b}, -- 8
				{ b, a, 0}, -- 9
				{ b,-a, 0}, --10
				{-b, a, 0}, --11
				{-b,-a, 0}, --12
			}
			
		it.polys={
				{2,9,11},
				{1,11,9},
				{1,3,7},
				{1,5,3},
				{2,4,6},
				{2,8,4},
				{3,10,12},
				{4,12,10},
				{11,7,8},
				{12,8,7},
				{9,6,5},
				{10,5,6},
				{1,7,11},
				{1,9,5},
				{2,11,8},
				{2,6,9},
				{4,8,12},
				{4,10,6},
				{3,12,7},
				{3,5,10}, 
 			}
			
--		it.strips={}
		
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

	geom.dodecahedron=function(it)
		it=it or {}
		
		local phi=(1+math.sqrt(5))/2
		local a=1/2
		local b=(1/phi)/2
		local c=(2-phi)/2
		
		it.verts={
				{ 0, a, c}, -- 1
				{ 0, a,-c}, -- 2
				{ 0,-a, c}, -- 3
				{ 0,-a,-c}, -- 4
				{ c, 0, a}, -- 5
				{ c, 0,-a}, -- 6
				{-c, 0, a}, -- 7
				{-c, 0,-a}, -- 8
				{ a, c, 0}, -- 9
				{ a,-c, 0}, --10
				{-a, c, 0}, --11
				{-a,-c, 0}, --12
				{ b, b, b}, --13
				{ b, b,-b}, --14
				{ b,-b, b}, --15
				{ b,-b,-b}, --16
				{-b, b, b}, --17
				{-b, b,-b}, --18
				{-b,-b, b}, --19
				{-b,-b,-b}, --20
			}
			
		it.polys={
			{5,7,17,1,13},
			{7,5,15,3,19},
			{6,8,20,4,16},
			{8,6,14,2,18},
			{14,9,13,1,2},
			{17,11,18,2,1},
			{20,12,19,3,4},
			{15,10,16,4,3},
			{9,10,15,5,13},
			{10,9,14,6,16},
			{11,12,20,8,18},
			{12,11,17,7,19},
		}
			
--		it.strips={}
		
		for i,p in pairs(it.polys) do geom.fix_poly_order(it,p) end
		return it
	end

	return geom
end


--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wgrd=require("wetgenes.grd")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wxml=require("wetgenes.simpxml")
local wzips=require("wetgenes.zips")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,geom_dae)
	local geom_dae=geom_dae or {}
	geom_dae.oven=oven
	
	geom_dae.modname=M.modname

	geom_dae.fps=24 -- assumed fps, need "smart" code to work out on load the minimum time between frames

	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=oven.gl
	local sheets=cake.sheets
	local fbs=cake.framebuffers

	local geom=oven.rebake("wetgenes.gamecake.spew.geom")
	local geoms=oven.rebake("wetgenes.gamecake.spew.geoms")


	function geom_dae.load(opts)
	

	if type(opts)=="string" then
		opts={filename=opts}
	end
	local s=wzips.readfile(opts.filename)
print("loaded ",#s,"bytes from "..opts.filename)
	local x=wxml.parse(s)

	--print("loaded ",wxml.unparse(x))

	local ids={}
	local symbols={}
	local function do_ids(t)
		for i=1,#t do local v=t[i]
			if type(v)=="table" then
				if v.id then ids[v.id]=v end
				if v.symbol then symbols[v.symbol]=v end
				do_ids(v)
			end
		end
	end
	do_ids(x)
	local function get_by_id(id)
		if id:sub(1,1) == "#" then
			id=id:sub(2)
		end
		local d=ids[id] or symbols[id] 
		return d
	end

	local function get_dat(id)
		if id:sub(1,1) == "#" then
			id=id:sub(2)
		end
		local d=ids[id] or symbols[id] 
		if type(d[0])=="string" and ( string.lower(d[0])=="source" or string.lower(d[0])=="float_array" or string.lower(d[0])=="name_array") then
			return d 
		else
			local t=wxml.descendent(d,"input")
			return get_dat(t.source)
		end
	end

	local function scan_nums(s)
		local a={}
		for w in string.gfind(s, "([^%s]+)") do
			local n=tonumber(w)
			if n then
				a[#a+1]=n
			else
				a[#a+1]=w
			end
		end	
		return a
	end

	local sources={}
	local function get_source(id)
		if sources[id] then return sources[id] end
		
		local d=get_dat(id)
		
		local a=wxml.descendent(d,"accessor")
		local it={}
		
		it.stride=tonumber(a.stride)
		it.names={}

		for i,v in ipairs( wxml.descendents(a,"param") ) do
			it.names[#it.names+1]=v.name
		end
		
		it.data=scan_nums( get_dat(a.source)[1] )
		
		sources[id]=it
		return it
	end
	
	local scene=wxml.descendent(x,"visual_scene") -- just grab the first scene and look for an armature
	local joints={}
	if scene then 
		local parse
		parse=function(nodes,joints)
			for i,v in ipairs(nodes) do
				local joint
				if type(v)=="table" then
					if	( string.lower(v[0]      or "")=="node"  )
					and	( string.lower(v["type"] or "")=="joint" ) then -- found a joint
						joint={name=v.name,id=v.id,sid=v.sid}
						joints[#joints+1]=joint
						local m=wxml.descendent(v,"matrix")
						if m then
							joint.matrix=scan_nums(m[1])
						end
					end
					parse(v,joint or joints) -- recurse
				end
			end
		end
		parse(scene,joints)
	end


	local joint_lookup={}
	local joint_flipidx={}
	local idx=1
	local recurse recurse=function(it) -- hand out joint idxs
		for i,v in ipairs(it) do
			v.idx=idx
			joint_lookup[v.name]=v --  and build lookup table by name or id or sid or numeric idx
			joint_lookup[v.id]=v
			joint_lookup[v.sid]=v
			joint_lookup[v.idx]=v
			idx=idx+1
		end
		for i,v in ipairs(it) do
			recurse(v)
		end
	end recurse(joints)


	local get_joint=function(name)
		return joint_lookup[name]
	end
	
	for i,v in pairs(joint_lookup) do
		local ln=v.name:sub(-1)
		if     ln=="L" then ln="R"
		elseif ln=="R" then ln="L" end
		ln=v.name:sub(1,-2)..ln
		local j=get_joint(ln)
		if j then
			joint_flipidx[v.idx]=j.idx
		end
	end

--	print(wstr.dump(joints))
	
	local aas=wxml.descendent(x,"library_animation_clips")
	local anims={}
	for ia,va in ipairs(aas or {}) do -- just check the top level for anims 
		if va[0]=="animation_clip" then
			local anim={name=va["name"],time_start=math.floor(tonumber(va["start"])*geom_dae.fps+0.5),time_end=math.floor(tonumber(va["end"])*geom_dae.fps+0.5)}
			anims[va.name]=anim
print(anim.name,anim.time_start,anim.time_end)
			for i,v in ipairs(va or {}) do -- just check the top level for anims 
				if v.url then v=get_by_id(v.url) end -- reference
				local d=wxml.descendent(v,"channel")
				if d and d.target then
					local target=wstr.split(d.target,"/")
					local joint=get_joint(target[1])
					local samp=get_by_id(d.source)
					local src={}
					for i,v in ipairs(samp) do
						if type(v)=="table" and v[0]=="input" then
							if     v.semantic=="INPUT" then -- time
								src.time=get_source(v.source)
							elseif v.semantic=="OUTPUT" then -- matrix
								src.matrix=get_source(v.source)
							elseif v.semantic=="INTERPOLATION" then -- tween
								src.tween=get_source(v.source)
							end
						end
					end
					if joint and src.time and src.matrix and src.tween then -- got some animation we can use
--print(joint.idx,target[1],src.time,src.matrix,src.tween)
						local frames={}
						for i=1,#src.time.data do
							local frame={}
							frame.time=src.time.data[i]
							frame.tween=src.tween.data[i]
							frame.matrix={}
							for j=1,16 do
								frame.matrix[j]=src.matrix.data[((i-1)*16)+j]
							end
							frames[math.floor(frame.time*geom_dae.fps+0.5)]=frame
						end
						anim[joint.idx]=frames
--test hax
--joint.frames=frames
	--					print(target[1],target[2],#anim)
					end
				end
			end
--			local t=get_by_id(id)
		end
	end
--	print(wstr.dump(joints))

--[[
for n,v in pairs(anims) do
	print(n)
	for n,v in pairs(v) do
		if type(n)=="number" then
			print("",n,#v)
		end
	end
end
]]

-- find skin weights so we can fetch them and apply to geoms later
	local ccs=wxml.descendent(x,"library_controllers")
	local skins={}
	local weights={}
	for i,v in ipairs(ccs or {}) do -- just check the top level for controlers
		if v[0]=="controller" then
			local skin=wxml.descendent(v,"skin")
			if skin and skin.source then
				local n=skin.source:sub(2) -- skip leading hash
				skins[n]=skin
				local dat={}
				weights[n]=dat
				
				local inputs={}
				local ws=wxml.descendent(skin,"vertex_weights")
				for i,l in ipairs( wxml.descendents(ws,"input") ) do
					local input={}
					input.idx=#inputs+1		
					input.semantic=l.semantic
					input.offset=tonumber(l.offset)
					input.source=get_source(l.source)
					inputs[input.idx]=input
					inputs[input.semantic:lower()]=input
				end

				local vc=scan_nums( wxml.descendent(v,"vcount")[1] )
				local vs=scan_nums( wxml.descendent(v,"v")[1] )
--print(wstr.dump(inputs))
				local vi=1
				for i,v in ipairs(vc) do
					local ws={}
					dat[#dat+1]=ws
					for i=1,v do
						local n=vs[vi] vi=vi+1
						local w=vs[vi] vi=vi+1
						local name=inputs.joint.source.data[n+1]
						local weight=inputs.weight.source.data[w+1]
						local joint=get_joint(name)
						ws[#ws+1]=joint.idx
						ws[#ws+1]=weight
					end
				end
--print(wstr.dump(dat))
			end
		end
	end 
--dprint(weights)


-- need to handle multiple geometries here...
	local its=geoms.new()
	
	its.anims=anims

	if joints[1] then
		its.joints=joints
	end

	local geo
	local t=wxml.descendent(x,"library_geometries")
	if t then
	for i=1,#t do local v=t[i]
		if v[0]=="geometry" then
			geo={}

			local weights=v.id and weights[v.id]

			geo.name=v.name
			geo.mesh=wxml.descendent(v,"mesh")


			local it=geom.new(it)
			it.name=v.name
			it.verts={}
			it.polys={}
			it.mats={}
			its[#its+1]=it

			--print("loading object named \""..geo.name.."\"")

			local polys={}
			for i,v in ipairs( wxml.descendents(geo.mesh,"polylist")) do -- handle each polylist chunk

				local p={}
				polys[#polys+1]=p
				
				p.material=v.material
				
				p.inputs={}
				
				p.stride=0
				for i,l in ipairs( wxml.descendents(v,"input") ) do
					local m={}
					m.idx=#p.inputs+1		
					m.semantic=l.semantic
					m.offset=tonumber(l.offset)
					m.source=get_source(l.source)
					if m.offset > p.stride then p.stride=m.offset end

					p.inputs[m.idx]=m
					p.inputs[m.semantic]=m -- easy lookup by name
				end
				p.stride=p.stride+1 -- this is how we guess this number, it will be one more than the inputs?
				p.vcount=scan_nums( wxml.descendent(v,"vcount")[1] )
				p.p=scan_nums( wxml.descendent(v,"p")[1] )
				
			--	dprint( p )

			end
			for i,v in ipairs( wxml.descendents(geo.mesh,"polygons")) do -- handle each polylist chunk

				local p={}
				polys[#polys+1]=p
				
				p.material=v.material
				
				p.inputs={}
				
				p.stride=0
				for i,l in ipairs( wxml.descendents(v,"input") ) do
					local m={}
					m.idx=#p.inputs+1		
					m.semantic=l.semantic
					m.offset=tonumber(l.offset)
					m.source=get_source(l.source)
					if m.offset > p.stride then p.stride=m.offset end

					p.inputs[m.idx]=m
					p.inputs[m.semantic]=m -- easy lookup by name
				end
				p.stride=p.stride+1 -- this is how we guess this number, it will be one more than the inputs?

				p.vcount={}
				p.p={}
				for ia,va in ipairs( wxml.descendents(v,"p")) do -- handle each polygon chunk

					local ns=scan_nums(va[1])
					
					table.insert(p.vcount,#ns)
					for ib=1,#ns do table.insert(p.p,ns[ib]) end
					
				end
				

			end

			for i,v in ipairs( wxml.descendents(geo.mesh,"triangles")) do -- handle each triangles chunk

				local p={}
				polys[#polys+1]=p
				
				p.material=v.material
				
				p.inputs={}
				
				p.stride=0
				for i,l in ipairs( wxml.descendents(v,"input") ) do
					local m={}
					m.idx=#p.inputs+1		
					m.semantic=l.semantic
					m.offset=tonumber(l.offset)
					m.source=get_source(l.source)
					if m.offset > p.stride then p.stride=m.offset end

					p.inputs[m.idx]=m
					p.inputs[m.semantic]=m -- easy lookup by name
				end
				p.stride=p.stride+1 -- this is how we guess this number, it will be one more than the inputs?

				p.vcount={}
				p.p={}
				for ia,va in ipairs( wxml.descendents(v,"p")) do -- handle each polygon triangle chunk

					local ns=scan_nums(va[1])
					
					for ib=1,#ns,3 do

						table.insert(p.vcount,3)

						table.insert(p.p,ns[ib+0])
						table.insert(p.p,ns[ib+1])
						table.insert(p.p,ns[ib+2])
					end
					
				end
				

			end


--			print("found poly list count "..#polys)

		-- turn dae split vertexs into a single vertex idx
			local vertex_idxs={}
			local function get_vertex_idx(xyz,nrm,uv,bone)
				local id=(xyz or "")..":"..(nrm or "")..":"..(uv or "")..":"..(bone or "")
				local idx=vertex_idxs[id]
				if not idx then
					idx=#vertex_idxs+1
					vertex_idxs[idx]=idx
					vertex_idxs[id]=idx
--					print("NEW",id)
				else
--					print(idx,id)
				end
				return idx
			end
			local material_idxs={}
			local function get_material_idx(id)

				if not id then return nil end
				local idx=material_idxs[id]
				if not idx then
					idx=#material_idxs+1
					material_idxs[idx]=idx
					material_idxs[id]=idx

					local mat={}
					local v=get_by_id(id)

					if v.target then v=get_by_id(v.target) end -- another level of redirection
					local m=get_by_id( v[1] and v[1].url )
					local sids={}
					local function do_sids(t)
						for i=1,#t do local v=t[i]
							if type(v)=="table" then
								if v.sid then sids[v.sid]=v else sids[v[0]]=v[1] end
								do_sids(v)
							end
						end
					end
					do_sids(m)

					mat.diffuse=  sids["diffuse"]   and scan_nums(sids["diffuse"][1]   ) or {1,1,1,1}
					mat.specular= sids["specular"]  and scan_nums(sids["specular"][1]  ) or {0,0,0,1}
					mat.shininess=sids["shininess"] and scan_nums(sids["shininess"][1] ) or {4}
					
					mat.diffuse[4]=1
					mat.specular[4]=1
					
					mat.idx=idx
					
					mat.name=v.name


--dprint(mat)

					it.mats[idx]=mat
				end
				return idx
			end
			
			local need_normals=false
			local need_tangents=true

			for ips,ps in ipairs(polys) do
			
--		print("found poly count "..#ps.vcount.." material "..ps.material)
				local off=1
				for ipc,pc in ipairs(ps.vcount) do
					local poly={}
					poly.mat=get_material_idx(ps.material) or 0
					for i=1,pc do -- each vertex 1tri or 2tri (quad)
						
						local inp=ps.inputs["VERTEX"]
						local xyz,xyzs
						if inp then
							xyz=ps.p[ off+(ps.stride*(i-1))+inp.offset ]
							xyzs=inp.source
						end
						
						local inp=ps.inputs["NORMAL"]
						local nrm,nrms
						if inp then
							nrm=ps.p[ off+(ps.stride*(i-1))+inp.offset ]
							nrms=inp.source
						else
							need_normals=true
						end

						local inp=ps.inputs["TEXCOORD"]
						local uv,uvs
						if inp then
							uv=ps.p[ off+(ps.stride*(i-1))+inp.offset ]
							uvs=inp.source
						end
												
						local w1=0 -- 0 means no bone weight (100% default)
						local w2=0
						local w3=0
						local w4=0
						if weights then
							local ws=weights[xyz+1]
							local calc=function(n,w)
								if n and w then
									n=get_joint(n)
									if n then
										return n.idx+(1-w) 	-- joint id + fractional weight 
															-- (id.0==100% id.25=75% id.5=50% id.75=25%)
									end
								end
								return 0
							end
							if ws then
								w1=calc(ws[1],ws[2])
								w2=calc(ws[3],ws[4])
								w3=calc(ws[5],ws[6])
								w4=calc(ws[7],ws[8])
							end
						end

-- blender fucks up the unique uv indexes, each is unique, so we merge matching ones here

						local vert=	{
											xyzs and xyzs.data[ (xyz*xyzs.stride) +1 ] or 0,
											xyzs and xyzs.data[ (xyz*xyzs.stride) +2 ] or 0,
											xyzs and xyzs.data[ (xyz*xyzs.stride) +3 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +1 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +2 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +3 ] or 0,
											
											uvs and uvs.data[ (uv*uvs.stride) +1 ] or 0,
											uvs and uvs.data[ (uv*uvs.stride) +2 ] or 0,
											
											0,0,0,0, -- blender fails to export tangents
											
											w1,w2,w3,w4 -- 4 bone ids and weights
										}

						local xyz_merge="["..vert[1]..","..vert[2]..","..vert[3].."]"						
						local nrm_merge="["..vert[4]..","..vert[5]..","..vert[6].."]"						
						local uv_merge ="["..vert[7]..","..vert[8].."],"..poly.mat
						local bone_merge ="["..vert[13]..","..vert[14]..","..vert[15]..","..vert[16].."]"

--						local idx=get_vertex_idx(xyz_merge,nrm_merge,uv_merge,bone_merge)
						local idx=get_vertex_idx(xyz,nrm,uv,bone_merge)
						
						it.verts[idx]=	vert

--print(idx,xyz,nrm,uv,bone_merge,vert[4],vert[5],vert[6])

						poly[#poly+1]=idx
						
					end
					off=off+ps.stride*pc

					it.polys[#it.polys+1]=poly
									
				end
				
			end
			
			if opts.mirror then
				local f=function(n)
					local i,f=math.modf(n)
					i=joint_flipidx[i] or i
					return i+f
				end
				geom.mirror(it,opts.mirror,function(it,dupe)
					if dupe then
						for i=13,16 do
							it[i]=f(it[i])
						end
					else
						local t={}
						for i=13,16 do
							if it[i] and it[i]~=0 then
								t[#t+1]=it[i]
							end
						end
						local m=#t
						for i=1,m do
							if f(t[i])~=t[i] then -- add mirrored (duplicate weight)
								t[#t+1]=f(t[i]) -- insert at end
							end
						end
						local m=0 -- so we need to calculate total weight
						for i=1,4 do local v=t[i]
							if v then
								local n,f=math.modf(v)
								m=m+(1-f)
							end
						end
						if m>0 then
							m=1/m -- and scale weights so they add up to 1
							for i=1,4 do
								if t[i] and t[i]~=0 then
									local n,f=math.modf(t[i])
									f=1-((1-f)*m)
									it[12+i]=n+f
								end
							end
						end
					end
				end)
				-- after mirroring I think it is best to rebuild all the normals/tangents
				need_normals=true
				need_tangents=true
			end
--			dprint(it.verts)
			if need_normals then
				geom.build_normals(it)
			end
			if need_tangents then
				geom.build_tangents(it)
			end
			
		end
	end
	end


	its.filter=opts.filter or {}
	
-- merge all objects into one
	its.anim_merge=function()

print("merging")

		its.grd_texmat=nil

		local tweaks={}
		
		if its.filter.bone_tweaks then -- per character bone tweaks that should be applied to the vertexes
			if its.joints then -- got a rig

					local recurse recurse=function(it,mp)

						if it.matrix then -- top level has no matrix
							
							local tweak=its.filter.bone_tweaks[it.name]
							if tweak then

								local inv=its.anim.rest[it.idx]:inverse( tardis.m4.new() )
								tweaks[it.idx]=tardis.m4.new()

							else

								tweaks[it.idx]=tardis.m4.new()

							end
						end

						for i,v in ipairs(it) do recurse(v,mp) end

					end	recurse(its.joints,tardis.m4.new():identity())

			end
		end

		local obj=its.anim_merged or geom.new()
		its.anim_merged=obj

		obj:clear_predraw()
		obj.verts={}
		obj.mats={}
		obj.polys={}
		obj.name="merged"
		

		local verts_idx=0
		local polys_idx=0
		local mats_map={}

-- merge all materials
		for i=1,#its do
			local it=its[i]
			if not its.filter.show_objects or its.filter.show_objects[ it.name ] then
			
				for im = 1 , #it.mats do local mat=it.mats[im]
					if not mats_map[ mat.name ] then 
					
						local m={}
						mats_map[ mat.name ]=m
						
						m.name=mat.name
						m.diffuse={unpack(mat.diffuse)}
						m.specular={unpack(mat.specular)}
						m.shininess={unpack(mat.shininess)}

					end
				end
			end
		end

-- reindex materials		
		local idx=1
		for n,v in pairs(mats_map) do
			v.idx=idx
			obj.mats[idx]=v
			idx=idx+1
		end

		for i=1,#its do
			local it=its[i]
			if not its.filter.show_objects or its.filter.show_objects[ it.name ] then
			
				-- build mat remap idx to idx table
				local mats_map_idx={}
				for im = 1 , #it.mats do local mat=it.mats[im]
					mats_map_idx[mat.idx]=mats_map[mat.name].idx
				end
			
				for iv = 1 , #it.verts do local vert=it.verts[iv]
					local v={unpack(vert)}
					obj.verts[ verts_idx+iv ]=v
				end
				
				for iv = 1 , #it.polys do local poly=it.polys[iv]
					local p={unpack(poly)}
					for i=1,#p do p[i]=p[i]+verts_idx end
					p.mat=mats_map_idx[poly.mat]
					obj.polys[ polys_idx+iv ]=p
				end
				
				verts_idx=verts_idx+#it.verts
				polys_idx=polys_idx+#it.polys
			
			end
		end

	end

	its.anim_setup=function()
	
		its.anim={}
		its.anim.rest={}
		its.anim.bones={}

		if its.joints then -- got a rig, build rest positions

				local recurse recurse=function(it,mp)

					if it.matrix then -- top level has no matrix
					
						local b=tardis.m4.new(it.matrix):transpose(tardis.m4.new()) -- transpose
										
						its.anim.rest[it.idx]=mp:product(b,tardis.m4.new())

						mp=its.anim.rest[it.idx]
					end

					for i,v in ipairs(it) do recurse(v,mp) end

				end	recurse(its.joints,tardis.m4.new():identity())

		end
		
		its.anim_merge()
	end
	its.anim_setup()

		
	its.anim_update=function(name,frame)
	
		its.anim.name=name
		its.anim.frame=frame
		its.anim.data=its.anims[name]

		if its.joints then -- got a rig

				local recurse recurse=function(it,mp)

					if it.matrix then -- top level has no matrix
					
						local b=tardis.m4.new(it.matrix)
						local frames=its.anim.data and its.anim.data[it.idx]
						
						local getm=function(f) return frames and frames[f] and frames[f].matrix end
						local i,f = math.modf(frame)
						local m1=tardis.m4.new(getm(i  ) or b)
						local m2=tardis.m4.new(getm(i+1) or b)
						
						
						local m=m1:lerp(m2,f) -- :transpose(tardis.m4.new()) -- transpose after lerp
						
						



						m:transpose(m)


						mp=mp:product(m,m)

						local tweak=its.filter.tweaks and its.filter.tweaks[it.name]
						if tweak then
						
							local rest=its.anim.rest[it.idx]
							local restinv=its.anim.rest[it.idx]:inverse(tardis.m4.new())
							local t=tardis.m4.new()

							its.anim.bones[it.idx]=tweak:product(restinv,t):product(rest)

							mp:product(t,mp)
						end


						local t=tardis.m4.new()				
						its.anim.bones[it.idx]=mp:product(its.anim.rest[it.idx]:inverse(t),t)



					end

					for i,v in ipairs(it) do recurse(v,mp) end

				end	recurse(its.joints,tardis.m4.new():identity())

		end

	end

	its.anim_draw=function()

		local it=its.anim_merged


--are we using too many uniforms?
--print(#it.mats,#its.anim.bones,#it.mats*2+#its.anim.bones*3)

		do
			if not its.gl_texmat then
				its.gl_texmat=assert(gl.GenTexture())
			end

			if not its.grd_texmat then

				local g=assert(wgrd.create(wgrd.FMT_U8_RGBA,64,2,1))
				its.grd_texmat=g
				
				local t,tl={},0
				for _,m in ipairs(it.mats) do
					local c1=m.diffuse
					t[tl+1]=c1[1]*255 t[tl+2]=c1[2]*255 t[tl+3]=c1[3]*255 t[tl+4]=c1[4]*255 tl=tl+4
				end
				g:pixels(0,0,tl/4,1,t)

				local t,tl={},0
				for _,m in ipairs(it.mats) do
					local c0=m.shininess
					local c2=m.specular
					t[tl+1]=c2[1]*255 t[tl+2]=c2[2]*255 t[tl+3]=c2[3]*255 t[tl+4]=c0[1] tl=tl+4
				end
				g:pixels(0,1,tl/4,1,t)

				gl.BindTexture( gl.TEXTURE_2D , its.gl_texmat )

				gl.TexImage2D(
					gl.TEXTURE_2D,
					0,
					gl.RGBA,
					g.width,
					g.height,
					0,
					gl.RGBA,
					gl.UNSIGNED_BYTE,
					g.data )
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)

			end
		end

		local bones=tardis.f32.new_v4(#its.anim.bones*3)
		do
			local t,tl={},0
			for _,m in ipairs(its.anim.bones) do
				t[tl+1]=m[1] t[tl+2]=m[5] t[tl+3]=m[9 ] t[tl+4]=m[13] tl=tl+4
				t[tl+1]=m[2] t[tl+2]=m[6] t[tl+3]=m[10] t[tl+4]=m[14] tl=tl+4
				t[tl+1]=m[3] t[tl+2]=m[7] t[tl+3]=m[11] t[tl+4]=m[15] tl=tl+4
			end
			tardis.f32.set(bones,t)
		end

		it.draw(it,"xyz_normal_mat_bone",function(p)

			gl.ActiveTexture(gl.TEXTURE0)
			gl.Uniform1i( p:uniform("tex_mat"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , its.gl_texmat )
			
			gl.Uniform4f( p:uniform("bones"), bones )

		end)
	
	end
		
		
		return its
	end


	return geom_dae
end

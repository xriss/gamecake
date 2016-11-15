--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wxml=require("wetgenes.simpxml")
local wzips=require("wetgenes.zips")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,geom_obj)
	local geom_obj=geom_obj or {}
	geom_obj.oven=oven
	
	geom_obj.modname=M.modname

	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=oven.gl
	local sheets=cake.sheets
	local fbs=cake.framebuffers

	local geom=oven.rebake("wetgenes.gamecake.spew.geom")


	function geom_obj.load(opts)
	
-- return value is a list of geoms (a list of 1)
	local geoms={}
	
	if type(opts)=="string" then
		opts={filename=opts}
	end
	local s=wzips.readfile(opts.filename)
print("loaded ",#s,"bytes from "..opts.filename)


--[[
	local x=wxml.parse(s)

	--print("loaded ",wxml.unparse(x))

	local ids={}
	local function do_ids(t)
		for i=1,#t do local v=t[i]
			if type(v)=="table" then
				if v.id then ids[v.id]=v end
				do_ids(v)
			end
		end
	end
	do_ids(x)
	local function get_by_id(id)
		if id:sub(1,1) == "#" then
			id=id:sub(2)
		end
		local d=ids[id]
		return d
	end

	local function get_dat(id)
		if id:sub(1,1) == "#" then
			id=id:sub(2)
		end
		local d=ids[id]
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
						joint={name=v.name}
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
			joint_lookup[v.name]=v --  and build lookup table by name or idx
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
	
	local aas=wxml.descendent(x,"library_animations")
	local anims={}
	for i,v in ipairs(aas or {}) do -- just check the top level for anims 
		if v[0]=="animation" then
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
					local anim={}
					for i=1,#src.time.data do
						local frame={}
						frame.time=src.time.data[i]
						frame.tween=src.tween.data[i]
						frame.matrix={}
						for j=1,16 do
							frame.matrix[j]=src.matrix.data[((i-1)*16)+j]
						end
						anim[i]=frame
					end
					joint.anim=anim
--					print(target[1],target[2],#anim)
				end
			end
--			local t=get_by_id(id)
		end
	end
--	print(wstr.dump(joints))

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
						ws[#ws+1]=name
						ws[#ws+1]=weight
					end
				end
--print(wstr.dump(dat))
			end
		end
	end 



-- need to handle multiple geometries here...
	local geoms={}
	
	if joints[1] then
		geoms.joints=joints
	end

	local geo
	local t=wxml.descendent(x,"library_geometries")
	for i=1,#t do local v=t[i]
		if v[0]=="geometry" then
			geo={}

			local weights=v.id and weights[v.id]

			geo.name=v.name
			geo.mesh=wxml.descendent(v,"mesh")


			local it=geom.new(it)
			it.verts={}
			it.polys={}
			it.mats={}
			geoms[#geoms+1]=it

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

--			print("found poly list count "..#polys)

		-- turn dae split vertexs into a single vertex idx
			local vertex_idxs={}
			local function get_vertex_idx(xyz,nrm,uv)
				local id=(xyz or "")..":"..(nrm or "")..":"..(uv or "")
				local idx=vertex_idxs[id]
				if not idx then
					idx=#vertex_idxs+1
					vertex_idxs[idx]=idx
					vertex_idxs[id]=idx
				end
				return idx
			end
			local material_idxs={}
			local function get_material_idx(id)
				local idx=material_idxs[id]
				if not idx then
					idx=#material_idxs+1
					material_idxs[idx]=idx
					material_idxs[id]=idx

					local mat={}
					local v=get_by_id(id)
					local m=get_by_id(v[1].url)
					local sids={}
					local function do_sids(t)
						for i=1,#t do local v=t[i]
							if type(v)=="table" then
								if v.sid then sids[v.sid]=v end
								do_sids(v)
							end
						end
					end
					do_sids(m)
					
					mat.diffuse=  sids["diffuse"]   and scan_nums(sids["diffuse"][1])   or {1,1,1,1}
					mat.specular= sids["specular"]  and scan_nums(sids["specular"][1])  or {0,0,0,1}
					mat.shininess=sids["shininess"] and scan_nums(sids["shininess"][1]) or {4}
					
					mat.diffuse[4]=1
					mat.specular[4]=1
					
					mat.idx=idx

--dprint(mat)

					it.mats[idx]=mat
				end
				return idx
			end
			
			local need_normals=false

			for ips,ps in ipairs(polys) do
			
--		print("found poly count "..#ps.vcount.." material "..ps.material)
				local off=1
				for ipc,pc in ipairs(ps.vcount) do
					local poly={}
					poly.mat=get_material_idx(ps.material)
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
						local idx=get_vertex_idx(xyz,nrm)
						it.verts[idx]=	{
											xyzs and xyzs.data[ (xyz*xyzs.stride) +1 ] or 0,
											xyzs and xyzs.data[ (xyz*xyzs.stride) +2 ] or 0,
											xyzs and xyzs.data[ (xyz*xyzs.stride) +3 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +1 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +2 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +3 ] or 0,
											0,0, -- uv
											w1,w2,w3,w4 -- 4 bone ids and weights
										}
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
						for i=9,12 do
							it[i]=f(it[i])
						end
					else
						local t={}
						for i=9,12 do
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
									it[8+i]=n+f
								end
							end
						end
					end
				end)
			end
--			dprint(it.verts)
			if need_normals then
				geom.build_normals(it)
			end
			
		end
	end

		
]]
		return geoms
	end


	return geom_obj
end

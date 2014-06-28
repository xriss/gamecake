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

M.bake=function(oven,geom_dae)
	local geom_dae=geom_dae or {}
	geom_dae.oven=oven
	
	geom_dae.modname=M.modname

	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=oven.gl
	local sheets=cake.sheets
	local fbs=cake.framebuffers

	local geom=oven.rebake("wetgenes.gamecake.spew.geom")


	function geom_dae.load(opts)
	

	if type(opts)=="string" then
		opts={filename=opts}
	end
	local s=wzips.readfile(opts.filename)
print("loaded ",#s,"bytes from "..opts.filename)
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
		if d[0]=="source" or d[0]=="float_array" then
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
			a[#a+1]=n
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

-- need to handle multiple geometries here...
	geoms={}

	local geo
	local t=wxml.descendent(x,"library_geometries")
	for i=1,#t do local v=t[i]
		if v[0]=="geometry" then
			geo={}
			geo.name=v.name
			geo.mesh=wxml.descendent(v,"mesh")


			local it={}
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

			print("found poly list count "..#polys)

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
					mat.shininess=sids["shininess"] and scan_nums(sids["shininess"][1]) or {0,0,0,1}
					
					mat.diffuse[4]=1
					
					mat.idx=idx

--dprint(mat)

					it.mats[idx]=mat
				end
				return idx
			end
			
			local need_normals=false

			for ips,ps in ipairs(polys) do
			
		print("found poly count "..#ps.vcount.." material "..ps.material)
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

						local idx=get_vertex_idx(xyz,nrm)
						it.verts[idx]=	{
											xyzs and xyzs.data[ (xyz*xyzs.stride) +1 ] or 0,
											xyzs and xyzs.data[ (xyz*xyzs.stride) +2 ] or 0,
											xyzs and xyzs.data[ (xyz*xyzs.stride) +3 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +1 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +2 ] or 0,
											nrms and nrms.data[ (nrm*nrms.stride) +3 ] or 0,
											0,0
										}
						poly[#poly+1]=idx
						
					end
					off=off+ps.stride*pc

					it.polys[#it.polys+1]=poly
									
				end
				
			end
			
--			dprint(it.verts)
			if need_normals then
				geom.build_normals(it)
			end
			
		end
	end

		
		
		return geoms
	end


	return geom_dae
end

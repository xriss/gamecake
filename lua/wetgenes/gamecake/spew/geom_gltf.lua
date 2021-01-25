--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wpath=require("wetgenes.path")
local wgrd=require("wetgenes.grd")
local wzips=require("wetgenes.zips")
local wgrd=require("wetgenes.grd")
local wgeom=require("wetgenes.gamecake.spew.geom")
local wgeoms=require("wetgenes.gamecake.spew.geoms")
local wstr=require("wetgenes.string")
local wpack=require("wetgenes.pack")
local wjson=require("wetgenes.json")
local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local function dprint(a) print(wstr.dump(a)) end




--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.export=function(env,...)
	local tab={...} ; for i=1,#tab do tab[i]=env[ tab[i] ] end
	return unpack(tab)
end




M.load=function(fname)

	local it_is_scary=function(uri)
		local scary=false
		if string.find(uri,"..",1,true) then scary=true end
		if string.find(uri,":",1,true)  then scary=true end
		if string.find(uri,"/",1,true)  then scary=true end
		if string.find(uri,"\\",1,true) then scary=true end
		return scary
	end

	local path=wpath.parse( fname )
	
	local input=path[1]..path[2]..path[3]..path[4]

	local gltf=M.parse( assert(wzips.readfile(fname),"failed to load "..fname) )
	
	for i=1,#gltf.buffers do
		local v=gltf.buffers[i]
		if type(v)=="table" then -- try and convert data to strings
			if v.uri then -- try and load file
				if not it_is_scary(v.uri) then

					local bname=path[1]..path[2]..v.uri
					gltf.buffers[i]=wzips.readfile(bname)

				end
			end
		end
	end

	if gltf.images then
		for i=1,#gltf.images do
			local v=gltf.images[i]
			if type(v)=="table" then -- try and convert data to strings
				if v.uri then -- try and load file

					if not it_is_scary(v.uri) then
						local iname=path[1]..path[2]..v.uri
						gltf.images[i]=wgrd.create():load_data(wzips.readfile(iname))
					end

				elseif v.bufferView then -- try and load data

					local view=gltf.bufferViews[v.bufferView+1]
					local buffer=gltf.buffers[view.buffer+1]
					local data=string.sub(buffer,(view.byteOffset or 0)+1,(view.byteOffset or 0)+view.byteLength+1)
					gltf.images[i]=wgrd.create():load_data(data)

				end
			end
		end
	end

	return gltf
end



M.info=function(gltf)

--	dprint(gltf)

	for n in pairs(gltf) do
		print(n, type(gltf[n])=="table" and "#"..#gltf[n] or gltf[n] )
	end
	

end

M.lookup_size={
	[5120]	=	1,
	[5121]	=	1,
	[5122]	=	2,
	[5123]	=	2,
	[5125]	=	4,
	[5126]	=	4,
}

M.lookup_fmt={
	[5120]	=	"s8",
	[5121]	=	"u8",
	[5122]	=	"s16",
	[5123]	=	"u16",
	[5125]	=	"u32",
	[5126]	=	"f32",
}

M.lookup_scale={
	[5120]	=	0x7f,
	[5121]	=	0xff,
	[5122]	=	0x7fff,
	[5123]	=	0xffff,
	[5125]	=	0xffffffff,
	[5126]	=	1,
}

M.lookup_count={
	["SCALAR"]	=	1,
	["VEC2"] 	=	2,
	["VEC3"] 	=	3,
	["VEC4"] 	=	4,
	["MAT2"] 	=	4,
	["MAT3"] 	=	9,
	["MAT4"] 	=	16,
}

M.accessor_to_table=function(gltf,id,scale)

	if not id then return nil end

	local tab={}

	local accessor=gltf.accessors[id+1]
	local view=gltf.bufferViews[accessor.bufferView+1]
	local buffer=gltf.buffers[view.buffer+1]
	local offset=(accessor.byteOffset or 0) + (view.byteOffset or 0)
	
	local bfmt=M.lookup_fmt[accessor.componentType]
	local bsize=M.lookup_size[accessor.componentType]
	local bcount=M.lookup_count[accessor.type]
	
-- need to deal with sparse matrix alignment for bytes and words...

	for ai=1,accessor.count do

		local aa=wpack.load_array(buffer,bfmt,offset,bcount*bsize)
		for bi=1,bcount do
			tab[ai*bcount-bcount+bi]=aa[bi]
		end
		
		offset=offset+(view.byteStride or (bcount*bsize) )
	
	end
	
	if scale then
		local bscale= scale / M.lookup_scale[accessor.componentType]
		if bscale~=1 then -- do not bother
			for i=1,#tab do
				tab[i]=tab[i]*bscale
			end
		end
	end
	
	return tab

end



M.parse=function(gltf)

	if type(gltf)=="string" then -- need to parse

		if string.sub(gltf,1,4) == "glTF" then -- split glb file

			local head=wpack.load_array(gltf,"u32",0,4*3)
			local base0=12
			local head0=wpack.load_array(gltf,"u32",base0,4*2)
			local base1=20+(math.ceil(head0[1]/4)*4)
			local head1=wpack.load_array(gltf,"u32",base1,4*2)

			local data=string.sub(gltf,base1+9,base1+9+head1[1])
			local json=string.sub(gltf,base0+9,base0+9+head0[1])

			gltf=wjson.decode(json)

			gltf.buffers=gltf.buffers or {}
			gltf.buffers[1]=data

		else
		
			gltf=wjson.decode(gltf)

		end
		
	end

	return gltf
end


local flipy=true

local vert_flipy=function(v)
	if flipy then
		v[2]=-v[2]
		v[5]=-v[5]
		v[10]=-v[10]
	end
	return v
end

-- this does not work, I guess it is going to be more complicated...
local m4_flipy=function(m,name)
	if flipy then

print(name)
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[1],m[2],m[3],m[4]))
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[5],m[6],m[7],m[8]))
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[9],m[10],m[11],m[12]))
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[13],m[14],m[15],m[16]))


--		m:inverse()

local x=m[13]*m[1] + m[14]*m[2] + m[15]*m[3]
local y=m[13]*m[5] + m[14]*m[6] + m[15]*m[7]
local z=m[13]*m[9] + m[14]*m[10] + m[15]*m[11]

print(string.format( "%8.4f %8.4f %8.4f", x,y,z))

m[1]=1
m[2]=0
m[3]=0
m[4]=0

m[5]=0
m[6]=1
m[7]=0
m[8]=0

m[9]=0
m[10]=0
m[11]=1
m[12]=0

m[13]=x
m[14]=-y
m[15]=z
m[16]=1

--		m:inverse()

		local a=m[2]
		local b=m[3]
		local c=m[7]

--		m[2]=m[5]
--		m[3]=m[9]
--		m[7]=m[10]

--		m[5]=a
--		m[9]=b
--		m[10]=c

--		m[2]=-m[2]
--		m[6]=-m[6]
--		m[10]=-m[10]
--		m[14]=-m[14]

--		m[5]=-m[5]
--		m[6]=-m[6]
--		m[7]=-m[7]
--		m[8]=-m[8]

--		m:inverse()

print("=")
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[1],m[2],m[3],m[4]))
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[5],m[6],m[7],m[8]))
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[9],m[10],m[11],m[12]))
print(string.format( "%8.4f %8.4f %8.4f %8.4f", m[13],m[14],m[15],m[16]))
print()

	end
	return m
end

local node_flipy=function(node)
	if flipy then
		if node.matrix then
			m4_flipy(node.matrix)
		end
		
		if node.translation then
			node.translation[2]=-node.translation[2]
		end

		if node.rotation then
			node.rotation[2]=-node.rotation[2]
			node.rotation[4]=-node.rotation[4]
		end
	end
	return node
end

local function value_flipy(value)
	if flipy then
		if value.path=="translation" then
			for i=0,#value.data-1,3 do
				value.data[i+2]=-value.data[i+2]
			end
		elseif value.path=="rotation" then
			for i=0,#value.data-1,4 do
				value.data[i+2]=-value.data[i+2]
				value.data[i+4]=-value.data[i+4]
			end
		end
	end
	return value
end



M.to_geoms=function(gltf)

	local objs=wgeoms.new()
	objs:reset()
	
	for midx=1,#gltf.meshes do
		local obj=wgeom.new()
		objs[#objs+1]=obj
	end

	objs.mats={}
	objs.textures={}

	if gltf.materials then
	for midx=1,#gltf.materials do -- copy gltf materials mostly as is
		local o=gltf.materials[midx]
		local m={}
		objs.mats[midx]=m
	
		m.name					=	o.name
		m.extensions			=	o.extensions
		m.extras				=	o.extras
		m.normalTexture			=	o.normalTexture
		m.occlusionTexture		=	o.occlusionTexture
		m.emissiveTexture		=	o.emissiveTexture
		m.emissiveFactor		=	o.emissiveFactor
		m.alphaMode				=	o.alphaMode
		m.alphaCutoff			=	o.alphaCutoff
		m.doubleSided			=	o.doubleSided

		if o.pbrMetallicRoughness then
			local p=o.pbrMetallicRoughness
			
			m.baseColorFactor			=	p.baseColorFactor
			m.baseColorTexture			=	p.baseColorTexture
			m.metallicFactor			=	p.metallicFactor
			m.roughnessFactor			=	p.roughnessFactor
			m.metallicRoughnessTexture	=	p.metallicRoughnessTexture
			m.pbr_extensions			=	p.extensions
			m.pbr_extras				=	p.extras

		end

-- build shader values

		local tex=function(opts)
			local it={}
			
			if opts.index then
				local t=gltf.textures[opts.index+1]
				local s=t.sampler and gltf.samplers[t.sampler+1] or {}
				local g=t.source and gltf.images[t.source+1] or {}
				
				it.grd=g

				it.magFilter=s.magFilter
				it.minFilter=s.minFilter
				it.wrapS=s.wrapS
				it.wrapT=s.wrapT
			end
		
			return it
		end

		m[1]={}	-- color texture
		m[2]={} -- pbr texture
		m[3]={} -- normal texture
		
		if m.baseColorTexture then
			m[1][1]=1 -- enable
			m[1].texture=tex(m.baseColorTexture)
		else
			m[1][1]=0 -- disable
		end
		
		m[1].color=m.baseColorFactor or {1,1,1,1}
		
		if m.metallicRoughnessTexture or m.occlusionTexture then -- these *should* be the same texture
			m[2][1]=m.occlusionTexture and ( m.occlusionTexture.strength or 1 ) -- enable
			m[2][2]=m.metallicFactor or 1-- enable
			m[2][3]=m.roughnessFactor or 1 -- enable
			m[2].texture=tex( m.metallicRoughnessTexture or m.occlusionTexture )
		else
			m[2][1]=0 -- disable
			m[2][2]=0 -- disable
			m[2][3]=0 -- disable
		end

		if m.normalTexture then
			m[3][1]=m.normalTexture.scale or 1 -- enable
			m[3].texture=tex( m.normalTexture )
		else
			m[3][1]=0 -- disable
		end


		if not objs.textures[1] then objs.textures[1]=m[1].texture end
		if not objs.textures[2] then objs.textures[2]=m[2].texture end
		if not objs.textures[3] then objs.textures[3]=m[3].texture end

	end
	end
	
	objs.nodes={}
	for nidx=1,#(gltf.nodes or {}) do
		local node=gltf.nodes[nidx]
		local it={}
		objs.nodes[nidx]=it
		it.nodeidx=nidx
		
		it.name=node.name
		
		node_flipy(node)
		
		if node.children then
			it.children={unpack(node.children)}
		end
		if node.mesh then
			it.mesh=objs[node.mesh+1]
		end

		it.matrix=node.matrix and M4{unpack(node.matrix)}
		if it.matrix then
			it.reset_matrix=M4{unpack(node.matrix)}
		end

		if not it.matrix then
			local t=node.translation and {unpack(node.translation)} or {0,0,0}
			local r=node.rotation and {unpack(node.rotation)} or {0,0,0,1}
			local s=node.scale and {unpack(node.scale)} or {1,1,1}
			it.trs={ t[1],t[2],t[3] , r[1],r[2],r[3],r[4] , s[1],s[2],s[3] }
			it.reset_trs={ t[1],t[2],t[3] , r[1],r[2],r[3],r[4] , s[1],s[2],s[3] }
		end

	end
	for nidx=1,#(objs.nodes or {}) do
		local node=objs.nodes[nidx]
		if node.children then
			for i=1,#node.children do
				node[i]=objs.nodes[ node.children[i]+1 ]
				node[i].parentidx=node.nodeidx
			end
		end
		node.children=nil
	end
	
	for sidx=1,#(gltf.scenes or {}) do
		local scene={}
		objs.scenes[sidx]=scene
		for i,n in ipairs( gltf.scenes[sidx].nodes ) do
			scene[i]=objs.nodes[n+1]
		end
	end
	objs.scene=objs.scenes[ (gltf.scene or 0) +1 ]


	objs.anims={}
	for aidx=1,#(gltf.animations or {}) do
	
		local anim={}
		objs.anims[aidx]=anim
		
		local animation=gltf.animations[aidx]
		
		local input=animation.samplers[1].input
		
		anim.name=animation.name
		if anim.name then objs.anims[anim.name]=anim end
		anim.keys=M.accessor_to_table(gltf,input)
		
		if input then
			local accessor=gltf.accessors[input+1]
			anim.min=accessor.min[1]
			anim.max=accessor.max[1]

			anim.values={}

			for cidx=1,#animation.channels do
				local channel=animation.channels[cidx]
				local sampler=animation.samplers[channel.sampler+1]
			
				if sampler.input==input then -- only animate shared input
					local value={}
					anim.values[#anim.values+1]=value
					value.node=channel.target.node+1
					value.path=channel.target.path
					value.data=M.accessor_to_table(gltf,sampler.output)
					value_flipy(value)
				end

			end

		end

	end
	objs.anim=objs.anims[ (gltf.animation or 0) +1 ]
	
-- assume file is exported with reset transform as current transform
	wgeoms.prepare(objs) -- so build world transform and we can then build our own inverse bone matrix

	objs.skins={}
	for sidx=1,#(gltf.skins or {}) do
		local skin=gltf.skins[sidx]
		local it={}
		objs.skins[sidx]=it
		it.inverse={}
		it.nodes={}
--		local d=M.accessor_to_table(gltf,skin.inverseBindMatrices) -- do nut trust these bones
		for i,v in ipairs(skin.joints) do
--[[
			local b=((i-1)*16)
			it.inverse[i]=m4_flipy( M4{
				d[b+ 1],d[b+ 2],d[b+ 3],d[b+ 4],
				d[b+ 5],d[b+ 6],d[b+ 7],d[b+ 8],
				d[b+ 9],d[b+10],d[b+11],d[b+12],
				d[b+13],d[b+14],d[b+15],d[b+16],
				} , objs.nodes[v+1].name )
]]				
			it.nodes[i]=objs.nodes[v+1]
			it.nodes[i].boneidx=i

			it.nodes[i].inverse=it.nodes[i].world:inverse() -- use world matrix
			it.inverse[i]=it.nodes[i].inverse -- as inverse bone

		end
	end
	
	
	for midx=1,#gltf.meshes do
	
		local mesh=gltf.meshes[midx]
		local obj=objs[midx] -- previously created
		
		obj.name=mesh.name

		obj.verts={}
		obj.polys={}
		obj.mats=objs.mats
		obj.textures=objs.textures
		
		local got_normals=false
		local got_tangents=false
		
		for pidx=1,#mesh.primitives do
		
			local vbase=#obj.verts
		
			local primitive=mesh.primitives[pidx]

			local tpos = M.accessor_to_table(gltf,primitive.attributes.POSITION)
			local tnrm = M.accessor_to_table(gltf,primitive.attributes.NORMAL)
			local ttan = M.accessor_to_table(gltf,primitive.attributes.TANGENT)
			local tuv  = M.accessor_to_table(gltf,primitive.attributes.TEXCOORD_0)

			local tj   = M.accessor_to_table(gltf,primitive.attributes.JOINTS_0)
			local tw   = M.accessor_to_table(gltf,primitive.attributes.WEIGHTS_0)
			
			if tnrm then got_normals=true end

			if tpos then -- must have a position
				for c=0,(#tpos/3)-1 do
					local bw={0,0,0,0} -- bone+(1-weight) combined
					for i=1,4 do
						if tj and tw then
							local j=tj[c*4+i]
							local w=tw[c*4+i]
							if w<0 then w=0 end
							if w>1 then w=1 end
							if w>0 then
								bw[i]=j+1+(1-w)
							end
						end
					end
					obj.verts[c+1+vbase]=vert_flipy( {
						tpos and tpos[c*3+1] or 0,tpos and tpos[c*3+2] or 0,tpos and tpos[c*3+3] or 0,
						tnrm and tnrm[c*3+1] or 0,tnrm and tnrm[c*3+2] or 0,tnrm and tnrm[c*3+3] or 0,
						tuv and tuv[c*2+1] or 0,tuv and tuv[c*2+2] or 0,
						ttan and ttan[c*4+1] or 0,ttan and ttan[c*4+2] or 0,ttan and ttan[c*4+3] or 0,ttan and ttan[c*4+4] or 0,
						bw[1],bw[2],bw[3],bw[4]
					} )
--print(bw[1],bw[2],bw[3],bw[4])
				end

				if (not primitive.mode) or (primitive.mode==4) then -- triangles
					if primitive.indices then
						local t=M.accessor_to_table(gltf,primitive.indices)
						for i=1,#t,3 do
							obj.polys[#obj.polys+1]={t[i]+1+vbase,t[i+1]+1+vbase,t[i+2]+1+vbase,mat=primitive.material and primitive.material+1}
						end
					else
						for i=vbase+1,#obj.verts,3 do
							obj.polys[#obj.polys+1]={i,i+1,i+2,mat=primitive.material and primitive.material+1}
						end
					end
				end
			end
		end

		if not got_normals then
			obj:build_normals()
		end
		
		if not got_tangents then
			obj:build_tangents()
		end
		
	end

--	dprint(objs.anims)
	
	return objs
end

M.view=function(gltf,oven)

	if not oven then
		local opts={
			disable_sounds=true,
			times=true, -- request simple time keeping samples
			width=800,	-- display basics
			height=600,
			name="gltf",
			title="gltf",
			fps=60,
		}
		-- setup oven with vanilla cake setup and save as a global value
		oven=require("wetgenes.gamecake.oven").bake(opts).preheat()
	end

	local view=oven.cake.views.create({
		parent=oven.cake.views.get(),
		mode="full",
		vx=oven.win.width,
		vy=oven.win.height,
		vz=oven.win.height*4,
		fov=0,
	})

	
	local main={}
	
	oven.next=main
	
	local gl=oven.gl
	local pack=require("wetgenes.pack")

	gl.program_source("geom_gltf",[[

#version 300 es
#version 330
#ifdef VERSION_ES
precision mediump float;
#endif

precision highp float;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform vec4  material_values[8];
uniform vec4  material_colors[8];

uniform mat4  bones[128];

varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_tangent;
varying vec3  v_bitangent;
varying vec3  v_pos;
varying vec2  v_texcoord;
varying float v_matidx;
varying vec4  v_bone;
varying vec4  v_value;


#ifdef VERTEX_SHADER

 
attribute vec4  a_color;
attribute vec3  a_vertex;
attribute vec3  a_normal;
attribute vec4  a_tangent;
attribute vec2  a_texcoord;
attribute float a_matidx;
attribute vec4  a_bone;


void main()
{
	mat4 v=modelview;

	if(a_bone[0]>0.0) //  some bone data
	{
		mat4 bm;
		mat4 bv;

		bm=bones[ int(a_bone[0]-1.0) ];
		bv=(1.0-fract(a_bone[0]))*(bm);

		if(a_bone[1]>0.0)
		{
			bm=bones[ int(a_bone[1]-1.0) ];
			bv+=(1.0-fract(a_bone[1]))*(bm);

			if(a_bone[2]>0.0)
			{
				bm=bones[ int(a_bone[2]-1.0) ];
				bv+=(1.0-fract(a_bone[2]))*(bm);
				
				if(a_bone[3]>0.0)
				{
					bm=bones[ int(a_bone[3]-1.0) ];
					bv+=(1.0-fract(a_bone[3]))*(bm);
				}
			}
		}

		
		v=modelview*bv;
	}

    gl_Position		=	projection * v * vec4(a_vertex,1.0);
	v_normal		=	normalize( mat3( v ) * a_normal );
	v_tangent		=	normalize( mat3( v ) * a_tangent.xyz );
	v_bitangent		=	normalize( cross( v_normal , v_tangent ) * a_tangent.w );
	v_bone			=	(a_bone);
	v_texcoord		=	a_texcoord;
	v_matidx		=	a_matidx;

	int idx=int( a_matidx );
	v_value = material_values[ idx ];
	v_color = material_colors[ idx ];
	
}


#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER


uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;


void main(void)
{

	vec4 t0=vec4(1.0,1.0,1.0,1.0);
	vec4 t1=vec4(0.5,0.5,0.5,0.5);
	vec3 t2=vec3(0.5,0.5,1.0);

	if( v_value.r > 0.0 )
	{
		t0 = v_value.r * texture2D(tex0, v_texcoord ).rgba;
	}

	if( v_value.g > 0.0 || v_value.b > 0.0 )
	{
		t1 = texture2D(tex1, v_texcoord ).rgba;
	}

	if( v_value.a > 0.0 )
	{
		t2 = v_value.a * texture2D(tex2, v_texcoord ).rgb;
	}
	
	t2=(t2-vec3(0.5,0.5,0.5))*2.0;

	vec3 n = normalize( (t2.x*v_bitangent) + (-t2.y*v_tangent) + (t2.z*v_normal) );

	gl_FragColor=vec4( ( t0 * v_color * (0.5+0.5*pow( n.z, 4.0 )) ).rgb , 1.0 ) ;

}


#endif //FRAGMENT_SHADER

]])


	local geom=oven.rebake("wetgenes.gamecake.spew.geom")
	local geoms=oven.rebake("wetgenes.gamecake.spew.geoms")

	local gg={}
	
	local objs=M.to_geoms(gltf)

--[[
	local hash={
		hair	={ 0.5 , 0.0 , 0.0 , 1.0 },
		skin	={ 1.0 , 0.5 , 0.0 , 1.0 },
		black	={ 0.0 , 0.0 , 0.0 , 1.0 },
		color1	={ 1.0 , 0.0 , 0.0 , 1.0 },
		color2	={ 0.0 , 0.0 , 1.0 , 1.0 },
		eye		={ 1.0 , 1.0 , 1.0 , 1.0 },
		iris	={ 0.0 , 0.0 , 0.5 , 1.0 },
		lips	={ 1.0 , 0.0 , 0.0 , 1.0 },
	}
	for i,mat in ipairs(objs.mats) do
		local c=hash[mat.name]
		if c then mat[1].color=c end
	end
]]

	for i,texture in ipairs(objs.textures) do
	
		local g=texture.grd

		texture.gl=assert(gl.GenTexture())

		if g then
		
			g=g:convert(wgrd.FMT_U8_RGBA)

			gl.BindTexture(gl.TEXTURE_2D, texture.gl)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.REPEAT)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.REPEAT)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)

			gl.TexImage2D(
				gl.TEXTURE_2D,
				0,
				gl.RGBA,
				g.width,
				g.height,
				0,
				gl.RGBA,
				gl.UNSIGNED_BYTE,
				g.data)
		end

	end


	local view_position=V3{0,200,0}
	local view_scale=V3{1,1,1}
	local view_rotation=Q4{0,0,0,1}:rotate(180,{0,0,1})
	local view_orbit=V3{0,0,0}

	local m=0
	for i,obj in ipairs(objs) do
		local r=geom.max_radius(obj)
		if r>m then m=r end
	end
	if m>0 then
		local s=256/m
		view_scale=V3{s,s,s}
	end
	
main.update=function()

	if objs.anim then
		objs.anim.time=(objs.anim.time or 0)+(1/60)
	end

	geoms.update(objs)

end

	local mstate={}

main.msg=function(m)
	view.msg(m) -- fix mouse coords
	
	if m.class=="mouse" then
	
		if m.keyname=="wheel_add" then
			if m.action==-1 then
				local s=view_scale[1]*1.10
				view_scale={s,s,s}
			end
		elseif m.keyname=="wheel_sub" then
			if m.action==-1 then
				local s=view_scale[1]*0.90
				view_scale={s,s,s}
			end
		elseif m.keyname=="left" then -- click
		
			if m.action==1 then		mstate={"left",m.x,m.y}
			elseif m.action==-1 then	mstate={}
			end
			
		elseif m.keyname=="right" then -- click

			if m.action==1 then		mstate={"right",m.x,m.y}
			elseif m.action==-1 then	mstate={}
			end

		else

			if m.action==0 then -- drag
				if mstate[1]=="left" then
					local rs=1/2
					view_orbit[1]=view_orbit[1]+(m.x-mstate[2])*rs
					view_orbit[2]=view_orbit[2]+(m.y-mstate[3])*rs
					
					while view_orbit[1] >  180 do view_orbit[1]=view_orbit[1]-360 end
					while view_orbit[1] < -180 do view_orbit[1]=view_orbit[1]+360 end

					while view_orbit[2] >  180 do view_orbit[2]= 180 end
					while view_orbit[2] < -180 do view_orbit[2]=-180 end
					
					mstate[2]=m.x
					mstate[3]=m.y
				elseif mstate[1]=="right" then

					view_position:add( V3{m.x-mstate[2],m.y-mstate[3],0 } )
					mstate[2]=m.x
					mstate[3]=m.y
				end
			end
		
		end
		
	
	end

--	dprint(m)

end


main.draw=function()
		
--  we want the main view to track the window size
	oven.win:info()
	
	view.vx=oven.win.width
	view.vy=oven.win.height
	view.vz=oven.win.height*4
	
	oven.cake.views.push_and_apply(view)
	oven.cake.canvas.gl_default() -- reset gl state
	
	gl.ClearColor(pack.argb4_pmf4(0xf008))
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
	

	gl.PushMatrix()
	gl.Translate(view.vx/2,view.vy/2,0)

	gl.Translate(view_position[1],view_position[2],view_position[3])
	gl.Scale(view_scale[1],view_scale[2],view_scale[3])
	gl.MultMatrix( tardis.q4_to_m4(view_rotation) )

	gl.Rotate( view_orbit[1] ,  0,-1, 0 )
	gl.Rotate( view_orbit[2] ,  1, 0, 0 )
	
--	gl.Enable(gl.DEPTH_TEST)
	gl.state.set({
		[gl.DEPTH_TEST]					=	gl.TRUE,
	})

	local pp=function(p)
	
		if objs.textures[3] then
			gl.ActiveTexture(gl.TEXTURE2)
			gl.BindTexture(gl.TEXTURE_2D, objs.textures[3].gl)
			gl.Uniform1i( p:uniform("tex2"), 2 )
		end
		
		if objs.textures[2] then
			gl.ActiveTexture(gl.TEXTURE1)
			gl.BindTexture(gl.TEXTURE_2D, objs.textures[2].gl)
			gl.Uniform1i( p:uniform("tex1"), 1 )
		end
		
		if objs.textures[1] then
			gl.ActiveTexture(gl.TEXTURE0)
			gl.BindTexture(gl.TEXTURE_2D, objs.textures[1].gl)
			gl.Uniform1i( p:uniform("tex0"), 0 )
		end
		
		local bones={}
		local skin=objs.skins[1] -- assume only one boned character
		if skin then
			local b=0
			for i,v in ipairs(skin.nodes) do
				if i <= 128 then -- shader only supports a maximum of 128 bones
					local bone=v.bone or M4()
					for i=1,16 do bones[b+i]=bone[i] end
					b=b+16
				end
			end
		end
		
		material_colors={}
		material_values={}

		local b=0
		for i=1,#objs.mats do
			local mat = objs.mats[i]
			
			if i <= 8 then -- shader only supports a maximum of 8 mats
			
				material_values[b+1]=mat[1][1] or 0
				material_values[b+2]=mat[2][1] or 0
				material_values[b+3]=mat[2][2] or 0
				material_values[b+4]=mat[3][1] or 0
				
				material_colors[b+1]=mat[1].color[1] or 1
				material_colors[b+2]=mat[1].color[2] or 1
				material_colors[b+3]=mat[1].color[3] or 0
				material_colors[b+4]=mat[1].color[4] or 1
			end

			b=b+4
		end

		gl.UniformMatrix4f( p:uniform("bones"), bones )

		gl.Uniform4f( p:uniform("material_colors"), material_colors )
		gl.Uniform4f( p:uniform("material_values"), material_values )
		
	end


	geoms.draw(objs,"geom_gltf",pp)
--	geoms.draw_bones(objs,"geom_gltf",pp)

	gl.Disable(gl.DEPTH_TEST)


	gl.PopMatrix()

	oven.cake.views.pop_and_apply()

end


	-- this will busy loop or hand back control depending on the system we are running on, eitherway opts.start will run next 
	return oven:serv()

end


M.bake=function(oven,geom_gltf)
	local geom_gltf=geom_gltf or {}
	
	for n in pairs(M) do geom_gltf[n]=M[n] end
	
	geom_gltf.oven=oven
	geom_gltf.modname=M.modname

	return geom_gltf
end

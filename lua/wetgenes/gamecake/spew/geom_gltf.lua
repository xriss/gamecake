--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wgrd=require("wetgenes.grd")
local wgeom=require("wetgenes.gamecake.spew.geom")
local wgeoms=require("wetgenes.gamecake.spew.geoms")
local wstr=require("wetgenes.string")
local wpack=require("wetgenes.pack")
local wjson=require("wetgenes.json")
local tardis=require("wetgenes.tardis")


local function dprint(a) print(wstr.dump(a)) end




--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.export=function(env,...)
	local tab={...} ; for i=1,#tab do tab[i]=env[ tab[i] ] end
	return unpack(tab)
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
		end

		gltf=wjson.decode(gltf)
		
	end

--[[
	for n in pairs(gltf) do
		local it=gltf[n]
		if type(it)=="table" and it[1] then -- if array
			for i=1,#it do
				local v=it[i]
				if type(v)=="table" then
					if v.name then it[v.name]=v end -- allow lookup by name or index
				end
			end
		end
	end
]]

	return gltf
end

M.to_geoms=function(gltf)

	local objs=wgeoms.new()
	objs:reset()
	
	objs.mats={}
	objs.textures={}

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

	for midx=1,#gltf.meshes do
	
		local mesh=gltf.meshes[midx]

		local obj=wgeom.new()
		objs[#objs+1]=obj

		obj.verts={}
		obj.polys={}
		obj.mats=objs.mats
		obj.textures=objs.textures
		
		for pidx=1,#mesh.primitives do
		
			local primitive=mesh.primitives[pidx]

			local tpos=primitive.attributes.POSITION  and M.accessor_to_table(gltf,primitive.attributes.POSITION)
			local tnrm=primitive.attributes.NORMAL    and M.accessor_to_table(gltf,primitive.attributes.NORMAL)
			local tuv=primitive.attributes.TEXCOORD_0 and M.accessor_to_table(gltf,primitive.attributes.TEXCOORD_0)

			if tpos then -- must have a position
				for i=1,#tpos,3 do
					local j=(((i-1)/3)*2)+1
					obj.verts[#obj.verts+1]={
						tpos and tpos[i] or 0,tpos and -tpos[i+1] or 0,tpos and tpos[i+2] or 0,
						tnrm and tnrm[i] or 0,tnrm and -tnrm[i+1] or 0,tnrm and tnrm[i+2] or 0,
						tuv and tuv[j] or 0,tuv and tuv[j+1] or 0
					}
				end
				
				if primitive.indices then
					local t=M.accessor_to_table(gltf,primitive.indices)
					for i=1,#t,3 do
						obj.polys[#obj.polys+1]={t[i]+1,t[i+1]+1,t[i+2]+1,mat=primitive.material+1}
					end
				else
					for i=1,#obj.verts,3 do
						obj.polys[#obj.polys+1]={i,i+1,i+2,mat=primitive.material+1}
					end
				end
			end
		end

--		obj:build_normals()
		obj:build_tangents()
	end


	for sidx=1,#gltf.scenes or {} do
		
		local scene={}
		objs.scenes[sidx]=scene
		
		local add_children
		add_children=function(parent,nodes)
			for i,n in ipairs(nodes) do
				local node=gltf.nodes[n+1]
				local it={}
				parent[#parent+1]=it
				it.name=node.name
				
				if node.mesh then
					it.mesh=objs[node.mesh+1]
				end

				it.matrix=node.matrix and {unpack(node.matrix)}
				if not it.matrix then
					local t=node.translation and {unpack(node.translation)} or {0,0,0}
					local r=node.rotation and {unpack(node.rotation)} or {0,0,0,1}
					local s=node.scale and {unpack(node.scale)} or {1,1,1}
					it.trs={ t[1],-t[2],t[3] , r[1],r[2],r[3],r[4] , s[1],s[2],s[3] }
				end
				add_children( it , node.children or {} )
			end
		end
		add_children( scene , gltf.scenes[sidx].nodes or {} )
	end
	objs.scene=objs.scenes[ (gltf.scene or 0) +1 ]
	
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

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform vec4  material_colors[8];
uniform vec4  material_values[8];

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

precision highp float;


 
attribute vec4  a_color;
attribute vec3  a_vertex;
attribute vec3  a_normal;
attribute vec4  a_tangent;
attribute vec2  a_texcoord;
attribute float a_matidx;
attribute vec4  a_bone;


void main()
{
    gl_Position		=	projection * modelview * vec4(a_vertex, 1.0);
	v_normal		=	normalize( mat3( modelview ) * a_normal );
	v_tangent		=	normalize( mat3( modelview ) * a_tangent.xyz );
	v_bitangent		=	normalize( cross( v_normal , v_tangent ) * a_tangent.w );
	v_bone			=	a_bone;
	v_color			=	a_color;
	v_texcoord		=	a_texcoord;
	v_matidx		=	a_matidx;
	
	
	v_value = material_values[ int( a_matidx ) ];
	
}


#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER

precision highp float;

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

	gl_FragColor= ( t0 * (0.5+0.5*pow( n.z, 4.0 )) ) ;
	gl_FragColor.a=1.0;

//	gl_FragColor= vec4((n*0.5)+vec3(0.5,0.5,0.5),1.0);

}


#endif //FRAGMENT_SHADER

]])


	local geom=oven.rebake("wetgenes.gamecake.spew.geom")
	local geoms=oven.rebake("wetgenes.gamecake.spew.geoms")

	local gg={}
	
	local objs=M.to_geoms(gltf)

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


	local obj_position={0,0,0}
	local obj_scale={1,1,1}

--[[
	local dx,dy,dz,dw=0,0,0,0
	for i,obj in ipairs(objs) do
		local x,y,z=geom.find_center(obj)
		dx=dx+x
		dy=dy+y
		dz=dz+z
		dw=dw+1
	end
	if dw>0 then
		dx=dx/dw
		dy=dy/dw
		dz=dz/dw
		obj_position={-dx,-dy,-dz}
	end
]]

	local m=0
	for i,obj in ipairs(objs) do
		local r=geom.max_radius(obj)
		if r>m then m=r end
	end
	if m>0 then
		local s=256/m
		obj_scale={s,s,s}
	end
	
	local pos=tardis.v3.new()
	local rot=tardis.q4.new():identity()

main.update=function()
end

	local mstate={}

main.msg=function(m)
	view.msg(m) -- fix mouse coords
	
--[[
	rot:rotate(0.1,{0,1,0})
	rot:normalize()
]]

	if m.class=="mouse" then
	
		if m.keyname=="left" then -- click
		
			if m.action==1 then		mstate={"left",m.x,m.y,tardis.q4.new(rot)}
			elseif m.action==-1 then	mstate={}
			end
			
		elseif m.keyname=="right" then -- click

			if m.action==1 then		mstate={"right",m.x,m.y,tardis.q4.new(rot)}
			elseif m.action==-1 then	mstate={}
			end

		else

			if m.action==0 then -- drag
				if mstate[1]=="left" then
					rot=tardis.q4.new(mstate[4])
					rot:rotate( (m.x-mstate[2]) /4 ,{0,1,0})
					rot:rotate( (m.y-mstate[3]) /4 ,{1,0,0})
					rot:normalize()
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
--	gl.Translate(pos)
	gl.MultMatrix( tardis.q4_to_m4(rot) )
--	gl.Rotate(rot,1,1,1)
		
	gl.PushMatrix()
	gl.Translate( obj_position[1] , obj_position[2] , obj_position[3] )
	gl.Scale( obj_scale[1] , obj_scale[2] , obj_scale[3] )

	gl.Enable(gl.DEPTH_TEST)
	geoms.draw(objs,"geom_gltf",function(p)
	
		if objs.textures[1] then
			gl.ActiveTexture(gl.TEXTURE0)
			gl.BindTexture(gl.TEXTURE_2D, objs.textures[1].gl)
		end
		
		if objs.textures[2] then
			gl.ActiveTexture(gl.TEXTURE1)
			gl.BindTexture(gl.TEXTURE_2D, objs.textures[2].gl)
		end
		
		if objs.textures[3] then
			gl.ActiveTexture(gl.TEXTURE2)
			gl.BindTexture(gl.TEXTURE_2D, objs.textures[3].gl)
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
			
--material_values[b+4]=0

			end
		
			b=b+4
		end

		gl.Uniform4f( p:uniform("material_colors"), material_colors )
		gl.Uniform4f( p:uniform("material_values"), material_values )

		gl.Uniform1i( p:uniform("tex0"), 0 )
		gl.Uniform1i( p:uniform("tex1"), 1 )
		gl.Uniform1i( p:uniform("tex2"), 2 )

	end)
	gl.Disable(gl.DEPTH_TEST)


	gl.PopMatrix()

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

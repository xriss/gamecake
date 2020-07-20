--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wgeom=require("wetgenes.gamecake.spew.geom")
local wstr=require("wetgenes.string")
local wpack=require("wetgenes.pack")
local wjson=require("wetgenes.json")


local function dprint(a) print(wstr.dump(a)) end




--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.export=function(env,...)
	local tab={...} ; for i=1,#tab do tab[i]=env[ tab[i] ] end
	return unpack(tab)
end





M.info=function(gltf)

	dprint(gltf)

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

	local objs={}

	for midx=1,#gltf.meshes do
	
		local mesh=gltf.meshes[midx]

		local obj=wgeom.new()
		objs[#objs+1]=obj

		obj.verts={}
		obj.mats={}
		obj.polys={}
		
		for midx=1,#gltf.materials do
			local material=gltf.materials[midx]
			obj.mats[midx]={
				name=material.name,
				diffuse=	material.pbrMetallicRoughness and 
							material.pbrMetallicRoughness.baseColorFactor  and
							{unpack(material.pbrMetallicRoughness.baseColorFactor)},
			}
		end

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

	end

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
	
	local main={}
	
	oven.next=main
	
	local gl=oven.gl
	local pack=require("wetgenes.pack")

	gl.program_source("geom_gltf",[[

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;
varying vec2  v_texcoord;


#ifdef VERTEX_SHADER

 
attribute vec4  a_color;
attribute vec3  a_vertex;
attribute vec3  a_normal;
attribute vec2  a_texcoord;
attribute float a_matidx;

void main()
{
    gl_Position = projection * modelview * vec4(a_vertex, 1.0);
    v_normal = normalize( mat3( modelview ) * a_normal );
	v_color=a_color;
	v_texcoord=a_texcoord;
}


#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER


uniform sampler2D tex;
vec3 d=vec3(0.0,0.0,-1.0);

void main(void)
{
	vec4 tc=vec4(0.0,0.0,0.0,1.0);
	
	vec3 n=normalize(v_normal);

	gl_FragColor= tc + (vec4(1.0,1.0,1.0,1.0)*pow( n.z, 4.0 )) ;

}


#endif //FRAGMENT_SHADER

]])


	local geom=oven.rebake("wetgenes.gamecake.spew.geom")


	local objs=M.to_geoms(gltf)


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
		for i,obj in ipairs(objs) do
			geom.adjust_position(obj,-dx,-dy,-dz)
		end
	end

	local m=0
	for i,obj in ipairs(objs) do
		local r=geom.max_radius(obj)
		if r>m then m=r end
	end
	if m>0 then
		for i,obj in ipairs(objs) do
			geom.adjust_scale( obj , 256 / m )
		end
	end
	
	local rot=0

main.update=function()
	rot=rot+1
end

main.draw=function()
		
	gl.ClearColor(pack.argb4_pmf4(0xf008))
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.PushMatrix()
	gl.Translate(400,300,0)
	gl.Rotate(rot,1,1,1)
	
			
	gl.Enable(gl.DEPTH_TEST)
	for i,obj in ipairs(objs) do
		geom.draw_polys(obj,"geom_gltf")
	end
	gl.Disable(gl.DEPTH_TEST)


	gl.PopMatrix()


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

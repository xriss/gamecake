--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


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
	local offset=accessor.byteOffset+view.byteOffset
	
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


M.view=function(gltf)

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
	local oven=require("wetgenes.gamecake.oven").bake(opts).preheat()
	
	local main={}
	
	oven.next=main
	
	local gl=oven.gl
	local pack=require("wetgenes.pack")

	if not gl.programs.geom_gltf then -- setup our special shaders
	
		gl.shaders.v_geom_gltf={
		source=[[

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;
varying vec2  v_texcoord;
varying float v_face;
 
void main()
{

	v_face=a_texcoord.x-1.0;

	float vx=mod(v_face,8.0);
//	float vy=floor(v_face/8.0);

    gl_Position = projection * modelview * vec4(a_vertex, 1.0);
    v_normal = normalize( mat3( modelview ) * a_normal );
	v_color=color;

	int a=int(clamp(a_texcoord.y-1.0,0.0,2.0));
	
	if(a==2) {			v_texcoord=vec2(	 (vx+1.0)/8.0	,	 0.0	);
	} else if(a==1) {	v_texcoord=vec2(	 (vx+0.5)/8.0	,	 1.0	);
	} else {			v_texcoord=vec2(	 (vx    )/8.0	,	 0.0	);
	}

}

	]]
}

		gl.shaders.f_geom_gltf={
		source=[[

//uniform sampler2D tex;

varying vec4  v_color;
varying vec4  v_color2;
varying vec3  v_normal;
varying vec3  v_pos;
//varying vec2  v_texcoord;
varying float v_face;


vec3 d=vec3(0.0,0.0,-1.0);

void main(void)
{
	vec4 tc=vec4(0.0,1.0,0.0,0.0);
/*
	if( floor(v_face+(1.0/4096.0)) == ceil(v_face-(1.0/4096.0)) )
	{
		tc=texture2D(tex, v_texcoord);
	}
*/

	vec3 n=normalize(v_normal);
	gl_FragColor= tc + (v_color*max( n.z, 0.25 ))*(1.0-tc.a) ;
	gl_FragColor.a=1.0;
}

	]]
}

		gl.programs.geom_gltf={
			vshaders={"v_geom_gltf"},
			fshaders={"f_geom_gltf"},
		}
	
	end


	local geom=oven.rebake("wetgenes.gamecake.spew.geom")


	local obj=geom.new()
	obj.verts={}
	obj.polys={}
	local t=M.accessor_to_table(gltf,gltf.meshes[1].primitives[1].attributes.POSITION)
	for i=1,#t,3 do
		obj.verts[#obj.verts+1]={t[i],t[i+1],t[i+2]}
	end
	local t=M.accessor_to_table(gltf,gltf.meshes[1].primitives[1].indices)
	for i=1,#t,3 do
		obj.polys[#obj.polys+1]={t[i]+1,t[i+1]+1,t[i+2]+1}
	end


--	local obj=geom.icosahedron()
--	print(obj:max_radius())
	obj:adjust_scale( 256 / obj:max_radius() )
	
	local rot=0

main.update=function()
	rot=rot+1
end

main.draw=function()


--  we want the main view to track the window size
--	oven.win:info()
--	view.vx=oven.win.width
--	view.vy=oven.win.height
--	view.vz=oven.win.height*4
	
--	views.push_and_apply(view)
--	canvas.gl_default() -- reset gl state
		
	gl.ClearColor(pack.argb4_pmf4(0xf008))
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.PushMatrix()
	gl.Translate(400,300,0)
	gl.Rotate(rot,1,1,1)
	
	
--	font.set(cake.fonts.get("Vera")) -- default font
--	font.set_size(16,0)
	
--	gui.draw()
		
	geom.draw_polys(obj,"geom_gltf")


	gl.PopMatrix()

--	views.pop_and_apply()
	
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

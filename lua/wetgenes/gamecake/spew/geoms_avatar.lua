--
-- (C) 2020 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local function dprint(a) print(wstr.dump(a)) end

-- helper functions for managing avatars from a base avatar model and animations

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.filename="/sync/kriss/blender/avatar/avatar.glb" -- try and use latest version


M.bake=function(oven,geoms_avatar)

	local wgeom=oven.rebake("wetgenes.gamecake.spew.geom")
	local wgeom_gltf=oven.rebake("wetgenes.gamecake.spew.geom_gltf")
	local wgeoms=oven.rebake("wetgenes.gamecake.spew.geoms")

	local gl=oven.gl

	geoms_avatar=geoms_avatar or {}
	for n,v in pairs(M) do geoms_avatar[n]=v end
	geoms_avatar.modname=M.modname

-- we DO have OpenGL access here

	function geoms_avatar.loads()

		if not io.open(geoms_avatar.filename,"r") then
			geoms_avatar.filename="data/gltf/avatar.glb"
		end -- the gc will close the file...

		geoms_avatar.gltf=wgeom_gltf.load(geoms_avatar.filename)
		geoms_avatar.objs=wgeom_gltf.to_geoms(geoms_avatar.gltf)

--dprint(avatar.gs)

	gl.program_source("avatar_gltf",[[

precision highp float;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform vec4  material_values[8];
uniform vec4  material_colors[8];

uniform mat4  bones[64];

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

	end
	

	function geoms_avatar.update()

		local objs=geoms_avatar.objs

		local bones={}
		local skin=objs.skins[1] -- assume only one boned character
		if skin then
			local b=0
			for i,v in ipairs(skin.nodes) do
				if i <= 64 then -- shader only supports a maximum of 64 bones
					local bone=v.bone or M4()
					for i=1,16 do bones[b+i]=bone[i] end
					b=b+16
				end
			end
		end

		local material_colors={}
		local material_values={}
		
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

		geoms_avatar.bones=bones
		geoms_avatar.material_colors=material_colors
		geoms_avatar.material_values=material_values

		if objs.anim then
			objs.anim.time=(objs.anim.time or 0)+(1/24)
		end
		wgeoms.update(objs)

	end

	function geoms_avatar.draw()

		local pp=function(p)

			gl.UniformMatrix4f( p:uniform("bones"), geoms_avatar.bones )

			gl.Uniform4f( p:uniform("material_colors"), geoms_avatar.material_colors )
			gl.Uniform4f( p:uniform("material_values"), geoms_avatar.material_values )
			
		end

		gl.Rotate( 90 ,  1, 0, 0 )
--		wgeoms.draw(geoms_avatar.objs,"avatar_gltf",pp)

		wgeom.draw(geoms_avatar.obj,"avatar_gltf",pp)
		
	end
	

	function geoms_avatar.rebuild(soul,name)

		for i,m in ipairs(geoms_avatar.objs.mats) do
		
			local v=soul.materials[m.name or ""]
			if v then
			
				m[1].color[1]=v.diffuse[1]
				m[1].color[2]=v.diffuse[2]
				m[1].color[3]=v.diffuse[3]
				m[1].color[4]=v.diffuse[4]
			
			end

		end

		local obj=wgeom.new()
		obj:reset()
		obj.mats=geoms_avatar.objs.mats
		geoms_avatar.obj=obj
		
		local bv=0
		local bp=0
		local show={}		
		for n,v in pairs(soul.parts) do
			show[v]=true
		end
		
		for _,o in ipairs(geoms_avatar.objs) do

			if o.name and show[o.name] then
			
				for iv,vv in ipairs(o.verts) do
					obj.verts[bv+iv]={ unpack(vv) }
				end

				for ip,vp in ipairs(o.polys) do
					local pp={}
					obj.polys[bp+ip]=pp
					for i,v in ipairs(vp) do pp[i]=v+bv end
					pp.mat=vp.mat
				end
				
				bv=bv+#o.verts
				bp=bp+#o.polys

			end
		end

	end


	function geoms_avatar.avatar(avatar)
	
		avatar=avatar or {}
		
		avatar.update=function()
		end
	
		avatar.draw=function()
		end
	
		return avatar
	end
	

	return geoms_avatar
end


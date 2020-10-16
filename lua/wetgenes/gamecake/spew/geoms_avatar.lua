--
-- (C) 2020 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wpack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")
local wgrd=require("wetgenes.grd")
local wgrdcanvas=require("wetgenes.grdcanvas")

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

	local textures=oven.cake.textures
	
	geoms_avatar.bonetexs={} -- animation converted into a shader texture
	

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
		
		print(#geoms_avatar.objs.anims)
		for i,v in ipairs(geoms_avatar.objs.anims) do
			print("avatar anim :",i,math.floor(v.min*24),math.floor(v.max*24),v.name)
		end
		geoms_avatar.objs.anim=geoms_avatar.objs.anims[1]
		

--dprint(avatar.gs)

		geoms_avatar.map=wgrd.create(wgrd.FMT_U8_RGBA_PREMULT,64,16,1)
		
		geoms_avatar.map:clear(0xffffffff)
		
		geoms_avatar.image=oven.cake.images.load("avatar/edit","avatar/edit",function() return geoms_avatar.map end)
		geoms_avatar.image.TEXTURE_WRAP_S		=	gl.CLAMP_TO_EDGE
		geoms_avatar.image.TEXTURE_WRAP_T		=	gl.CLAMP_TO_EDGE
		geoms_avatar.image.TEXTURE_MIN_FILTER	=	gl.LINEAR
		geoms_avatar.image.TEXTURE_MAX_FILTER	=	gl.LINEAR


--		oven.cake.images.bind(geoms_avatar.image)


		local sl=wgeoms.get_skeleton_glsl(geoms_avatar.objs)
print(sl)

	gl.program_source("avatar_gltf",[[

precision highp float;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform vec4  material_values[8];
uniform vec4  material_colors[8];

uniform sampler2D fixbones;
uniform sampler2D texbones;

uniform float animframe;

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

mat4 fixbone(int bidx,int frame)
{
	return transpose( mat4(
		texelFetch(fixbones,ivec2(bidx*3+0,frame),0),
		texelFetch(fixbones,ivec2(bidx*3+1,frame),0),
		texelFetch(fixbones,ivec2(bidx*3+2,frame),0),
		vec4(0.0,0.0,0.0,1.0)) );
}

mat4 texbone(int bidx,int frame)
{
	return transpose( mat4(
		texelFetch(texbones,ivec2(bidx*3+0,frame),0),
		texelFetch(texbones,ivec2(bidx*3+1,frame),0),
		texelFetch(texbones,ivec2(bidx*3+2,frame),0),
		vec4(0.0,0.0,0.0,1.0)) )*fixbone(bidx, 0 );
}

]]..sl..[[

mat4 getbone(int bidx)
{
	return skeleton( bidx , int(animframe) )*fixbone(bidx, 1 );
//	return bones[ bidx ];
}


void main()
{
	mat4 v=modelview;

	if(a_bone[0]>0.0) //  some bone data
	{
		mat4 bm;
		mat4 bv;

		bm=getbone( int(a_bone[0]-1.0) );
		bv=(1.0-fract(a_bone[0]))*(bm);

		if(a_bone[1]>0.0)
		{
			bm=getbone( int(a_bone[1]-1.0) );
			bv+=(1.0-fract(a_bone[1]))*(bm);

			if(a_bone[2]>0.0)
			{
				bm=getbone( int(a_bone[2]-1.0) );
				bv+=(1.0-fract(a_bone[2]))*(bm);
				
				if(a_bone[3]>0.0)
				{
					bm=getbone( int(a_bone[3]-1.0) );
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

	if( v_value.r > 0.0 )
	{
		vec2 uv=clamp( v_texcoord + vec2( pow( n.z, 4.0 )-0.5 ,0.0) , vec2(0.0,0.0) , vec2(1.0,1.0) ) ;

		t0 = v_value.r * texture2D(tex0, uv ).rgba;
	}

	gl_FragColor=vec4( ( t0 * v_color ).rgb , 1.0 ) ;

}


#endif //FRAGMENT_SHADER

]])

		geoms_avatar.build_texture_anims()
		geoms_avatar.build_texture_tweak()

	end
	
	function geoms_avatar.build_texture_tweak()
	
		local width=0
		local height=2
		
		local fs={}
		
		local ts=wgeoms.get_anim_tweaks(geoms_avatar.objs)
		width=(#ts/8)
		
		local b=#fs
		for i=1,#ts do -- copy
			fs[b+i]=ts[i]
		end
		
		local d=wpack.save_array(fs,"f32")

		if geoms_avatar.fixtex then
		
			geoms_avatar.fixtex.gl_data=d
			geoms_avatar.fixtex:upload()

		else
			geoms_avatar.fixtex=textures.create({
				gl_data=d,
				gl_width=width,
				gl_height=height,
				gl_internal=gl.RGBA32F,
				gl_format=gl.RGBA,
				gl_type=gl.FLOAT,
			})
		end

	end

	function geoms_avatar.build_texture_anims()
	
		local objs=geoms_avatar.objs
		
		for aidx,anim in ipairs(objs.anims) do
			objs.anim=anim


			local width=0
			local height=#anim.keys
			
			local fs={}
			for kidx=1,#anim.keys do
			
				local ts=wgeoms.set_anim_frame(objs,kidx)
				width=(#ts/4)
				
				local b=#fs
				for i=1,#ts do -- copy
					fs[b+i]=ts[i]
				end

			end
			
			print(anim.name,#fs)
			
			geoms_avatar.bonetexs[anim.name]=textures.create({
				id="avatar/bonetexs/"..anim.name,
				gl_data=wpack.save_array(fs,"f32"),
				gl_width=width,
				gl_height=height,
				gl_internal=gl.RGBA32F,
				gl_format=gl.RGBA,
				gl_type=gl.FLOAT,
			})

		end
		
	end

	function geoms_avatar.update(soul)

		local objs=geoms_avatar.objs

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

		local material_colors={}
		local material_values={}
		
		local b=0
		for i=1,#objs.mats do
			local mat = objs.mats[i]
			
			if i <= 8 then -- shader only supports a maximum of 8 mats
			
				material_values[b+1]=1--mat[1][1] or 0
				material_values[b+2]=0--mat[2][1] or 0
				material_values[b+3]=0--mat[2][2] or 0
				material_values[b+4]=0--mat[3][1] or 0
				
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


		objs.anim=objs.anims[soul.pose] or objs.anims[1]
		if objs.anim then
			objs.anim.time=(objs.anim.time or 0)+(1/60)
		end
		wgeoms.update(objs)
		
--		geoms_avatar.rebuild(soul)

	end

	function geoms_avatar.draw()

		local pp=function(p)
		
			gl.ActiveTexture(gl.TEXTURE2)
			geoms_avatar.fixtex:bind()
			gl.Uniform1i( p:uniform("fixbones"), 2 )

			gl.ActiveTexture(gl.TEXTURE1)
			geoms_avatar.bonetexs[geoms_avatar.objs.anim.name]:bind()
			gl.Uniform1i( p:uniform("texbones"), 1 )

			gl.ActiveTexture(gl.TEXTURE0)
			oven.cake.images.bind(geoms_avatar.image)
			gl.Uniform1i( p:uniform("tex0"), 0 )


			gl.Uniform1f( p:uniform("animframe"), geoms_avatar.objs.anim.frame )

			gl.UniformMatrix4f( p:uniform("bones"), geoms_avatar.bones )

			gl.Uniform4f( p:uniform("material_colors"), geoms_avatar.material_colors )
			gl.Uniform4f( p:uniform("material_values"), geoms_avatar.material_values )
			
		end

		gl.Scale( -1, 1, 1 )		
		gl.Rotate( 90 ,  1, 0, 0 )
	
--	wgeoms.draw(geoms_avatar.objs,"avatar_gltf",pp)

		wgeom.draw(geoms_avatar.obj,"avatar_gltf",pp)
		
	end
	

	function geoms_avatar.rebuild(soul,name)

		local clamp=function(a) if a<0 then a=0 elseif a>255 then a=255 end return math.floor(a) end
		local rgb=function(r,g,b)
			return          255  * 256*256*256 + 
					clamp(r*255) * 256*256 + 
					clamp(g*255) * 256 + 
					clamp(b*255)
		end
		geoms_avatar.map:clear(0xffffffff)
		for idx,name in ipairs({
			"black",
			"skin",
			"lips",
			"hair",
			"iris",
			"eye",
			"red",
			"orange",
			"yellow",
			"green",
			"blue",
			"cyan",
			"pink",
			"brown",
			"grey",
			"white",
		}) do
			local material=soul.materials[name]
			if material and material.ramp then

				local keys={}
				for i,v in ipairs(material.ramp) do
					local cr,cg,cb,ca=wpack.argb8_b4(v)
					keys[i]={
						argb=wpack.b4_argb8(cr,cg,cb,0xff),
						value=ca/255,
					}
				end
				local g=geoms_avatar.map:clip( 0,idx-1,0, 64,1,1 )
				wgrdcanvas.cmap_ramp(g,keys)
			end
		end
		oven.cake.images.unload(geoms_avatar.image) -- force a reload



		local obj=wgeom.new()
		local show={}		
		for n,v in pairs(soul.parts) do
			if type(v)=="table" then
				for i,n in ipairs(v) do
					show[n]=true
				end
			else
				show[v]=true
			end
		end
		
		for _,o in ipairs(geoms_avatar.objs) do
			if show[ o.name ] then
				obj:merge_from(o)
			end
		end
		geoms_avatar.obj=obj

-- tweak base positions of bones
--[[
		wgeoms.reset_anim( geoms_avatar.objs )
		for i,node in ipairs( geoms_avatar.objs.nodes ) do
			if node.name then
				local tweak
				local sx=1
				if node.name:sub(-2,-1)==".L" then
					sx=1
					tweak=soul.tweaks[node.name:sub(1,-3)]
				elseif node.name:sub(-2,-1)==".R" then
					sx=-1
					tweak=soul.tweaks[node.name:sub(1,-3)]
				else
					tweak=soul.tweaks[node.name]
				end
				if tweak then
					if node.trs then
						node.trs[ 1]=node.trs[ 1]+tweak.translate[ 1]*sx
						node.trs[ 2]=node.trs[ 2]+tweak.translate[ 2]
						node.trs[ 3]=node.trs[ 3]+tweak.translate[ 3]

						local qa=Q4( node.trs[4] , node.trs[5] , node.trs[6] , node.trs[7] )
						local qb=Q4(0,0,0,1)
						
						qb:rotate(tweak.rotate[1],{1,0,0})
						qb:rotate(tweak.rotate[2],{0,sx,0})
						qb:rotate(tweak.rotate[3],{0,0,sx})
						
						tardis.q4_product_q4(qa,qb)

						node.trs[ 4]=qa[1]
						node.trs[ 5]=qa[2]
						node.trs[ 6]=qa[3]
						node.trs[ 7]=qa[4]

						node.trs[ 8]=node.trs[ 8]*tweak.scale[1]
						node.trs[ 9]=node.trs[ 9]*tweak.scale[2]
						node.trs[10]=node.trs[10]*tweak.scale[3]
					end
				end
			end
		end
		wgeoms.prepare( geoms_avatar.objs )

		local bones={}
		local skin=geoms_avatar.objs.skins[1] -- assume only one boned character
		if skin then
			local b=0
			for i,v in ipairs(skin.nodes) do
				if i <= 128 then -- shader only supports a maximum of 66 bones
					local bone=v.bone or M4()
					for i=1,16 do bones[b+i]=bone[i] end
					b=b+16
				end
			end
		end
		obj:adjust_by_bones(bones)
]]

-- prepare tweak values
		for i,node in ipairs( geoms_avatar.objs.nodes ) do
			if node.name then
				local tweak
				local sx=1
				if node.name:sub(-2,-1)==".L" then
					sx=1
					tweak=soul.tweaks[node.name:sub(1,-3)]
				elseif node.name:sub(-2,-1)==".R" then
					sx=-1
					tweak=soul.tweaks[node.name:sub(1,-3)]
				else
					tweak=soul.tweaks[node.name]
				end
				if tweak then
						node.tweak={}
						node.tweak[ 1]=tweak.translate[ 1]*sx
						node.tweak[ 2]=tweak.translate[ 2]
						node.tweak[ 3]=tweak.translate[ 3]

						local qa=Q4(0,0,0,1)
						
						qa:rotate(tweak.rotate[1],{1,0,0})
						qa:rotate(tweak.rotate[2],{0,sx,0})
						qa:rotate(tweak.rotate[3],{0,0,sx})

						node.tweak[ 4]=qa[1]
						node.tweak[ 5]=qa[2]
						node.tweak[ 6]=qa[3]
						node.tweak[ 7]=qa[4]

						node.tweak[ 8]=tweak.scale[1]
						node.tweak[ 9]=tweak.scale[2]
						node.tweak[10]=tweak.scale[3]
				end
			end
		end

		geoms_avatar.build_texture_tweak()

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


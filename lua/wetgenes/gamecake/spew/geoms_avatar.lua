--
-- (C) 2020 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")
local rnd64k=require("wetgenes"):export("rnd64k")

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

-- the material names of an avatar in texture order
M.material_names={
	"white",
	"grey",
	"black",
	"skin",
	"lips",
	"hair",
	"iris",
	"eye",
	"head1",
	"head2",
	"body1",
	"body2",
	"foot1",
	"foot2",
	"hand1",
	"hand2",
}

-- the available mesh names of an avatar
-- this is increased after loading a glb to include all meshes
M.mesh_names={

	{"hat_baseball",		group=1,									},
	{"hat_kerchief",		group=1,									},
	{"hat_pirate",			group=1,									},
	{"hat_greek_helmet",	group=1,									},

	{"hair_base",			group=0,									},
	{"hair_basic",			group=1,									},
	{"hair_spiky",			group=1,									},
	{"hair_back",			group=1,									},
	{"hair_bob",			group=1,									},
	{"hair_flattop",		group=1,									},
	{"hair_shoulder",		group=1,									},
	{"hair_afro",			group=1,									},
	{"hair_quiff",			group=1,									},
	{"hair_hedgehog",		group=1,									},
	{"hair_top_spiked",		group=1,									},
	{"hair_moflop_left",	group=1,									},
	{"hair_goth",			group=1,									},
	{"hair_mess",			group=1,									},

	{"hair_fringe",			group=2,									},
	{"hair_bunches",		group=2,									},
	{"hair_pigtails",		group=2,									},

	{"head_base",			group=1,									},
	{"head_skull",			group=0,									},
	{"head_robot",			group=0,									},
	{"head_pill",			group=0,									},

	{"eyebrow_base",		group=1,									},
	{"eyebrow_block",		group=1,									},
	{"eyebrow_robot",		group=0,									},

	{"eye_base",			group=1,									},
	{"eye_lash",			group=1,									},
	{"eye_robot",			group=0,									},

	{"eyeball_base",		group=1,									},
	{"eyeball_cat",			group=1,									},
	{"eyeball_robot",		group=0,									},

	{"eyewear_glasses",		group=1,									},

	{"mouth_base",			group=1,									},
	{"mouth_jaw",			group=0,									},
	{"mouth_robot",			group=0,									},

	{"beard_goatbraid",		group=1,									},
	{"beard_goatee",		group=1,									},
	{"beard_tash",			group=1,									},
	{"beard_whiskers",		group=1,									},

	{"ear_base",			group=1,									},
	{"ear_bunny",			group=0,									},
	{"ear_robot",			group=0,									},

	{"nose_base",			group=1,									},
	{"nose_piggy",			group=0,									},
	{"nose_clown",			group=0,									},
	{"nose_robot",			group=0,									},

	{"body_belly",			group=1,									},
	{"body_belly_boob",		group=1,									},
	{"body_bone",			group=0,									},
	{"body_boob",			group=1,									},
	{"body_chest",			group=1,									},
	{"body_tshirt",			group=1,									},
	{"body_tshirt_boob",	group=1,									},
	{"body_bodice",			group=1,									},
	{"body_vest",			group=1,									},
	{"body_robot",			group=0,									},

	{"body_printer_support",group=0,									},
	{"body_tie",			group=0,									},
	{"body_tie_belly",		group=0,									},
	{"body_tie_boob",		group=0,									},
	{"body_coat",			group=0,									},
	{"body_coat_boob",		group=0,									},

	{"hand_base",			group=1,									},
	{"hand_bone",			group=0,									},
	{"hand_robot",			group=0,									},

	{"tail_bunny",			group=0,									},
	{"tail_devil",			group=0,									},

	{"foot_bare",			group=1,									},
	{"foot_bone",			group=0,									},
	{"foot_slipper",		group=1,									},
	{"foot_flipflop",		group=1,									},
	{"foot_shoe",			group=1,									},
	{"foot_boot",			group=1,									},
	{"foot_heel",			group=1,									},
	{"foot_robot",			group=0,									},

	{"item_hammer",			group=1,									},
	{"item_shield",			group=1,									},
	{"item_shortsword",		group=1,									},
}

-- the base part names of an avatar
M.part_names={
	"hat",
	"hair",
	"head",
	"eye",
	"eyeball",
	"eyebrow",
	"eyewear",
	"ear",
	"nose",
	"mouth",
	"beard",
	"body",
	"hand",
	"tail",
	"foot",
	"item",
}

-- the base part names of an avatar and current number of dropdowns for each
M.part_counts={
	hat=1,
	hair=3,
	head=1,
	eye=1,
	eyeball=1,
	eyebrow=1,
	eyewear=1,
	ear=1,
	nose=1,
	mouth=1,
	beard=1,
	body=2,
	hand=1,
	tail=1,
	foot=1,
	item=1,
}

-- the tweak part names of an avatar
M.tweak_names={
	"hat",
	"hair",
	"head",
	"eyebrow",
	"eye",
	"eyeball",
	"ear",
	"nose",
	"cheek",
	"mouth",
	"body",
	"boob",
	"belly",
	"hand",
	"tail",
	"foot",
	"item",
}

-- the animation names
M.pose_names={
	"breath",
	"walk",
	"relax",
	"fist",
	"reset",
}


M.simp_to_ramp=function(cr,cg,cb,ca)
	
	if cr<0   then cr=0   end
	if cr>255 then cr=255 end
	if cg<0   then cg=0   end
	if cg>255 then cg=255 end
	if cb<0   then cb=0   end
	if cb>255 then cb=255 end
	if ca<0   then ca=0   end
	if ca>255 then ca=255 end

	local crl=cr-ca
	local cgl=cg-ca
	local cbl=cb-ca
	
	if crl<0   then crl=0   end
	if crl>255 then crl=255 end
	if cgl<0   then cgl=0   end
	if cgl>255 then cgl=255 end
	if cbl<0   then cbl=0   end
	if cbl>255 then cbl=255 end
	
	local crh=cr+ca
	local cgh=cg+ca
	local cbh=cb+ca
	
	if crh<0   then crh=0   end
	if crh>255 then crh=255 end
	if cgh<0   then cgh=0   end
	if cgh>255 then cgh=255 end
	if cbh<0   then cbh=0   end
	if cbh>255 then cbh=255 end
	
	local ramp={}
	ramp[1]=wpack.b4_argb8(crl,cgl,cbl,0x00)
	ramp[2]=wpack.b4_argb8(cr,cg,cb,0x80)
	ramp[3]=wpack.b4_argb8(crh,cgh,cbh,0xff)

	return ramp
end


M.ramp_to_simp=function(ramp)

	local crl,cgl,cbl,cal=wpack.argb8_b4( ramp[1] or 0x00000000 )
	local cr,cg,cb,ca=wpack.argb8_b4( ramp[2] or 0x00000000 )
	local crh,cgh,cbh,cah=wpack.argb8_b4( ramp[3] or 0x00000000 )

	ca=0

	if cr-crl > ca then ca=cr-crl end
	if cg-cgl > ca then ca=cg-cgl end
	if cb-cbl > ca then ca=cb-cbl end

	if crh-cr > ca then ca=crh-cr end
	if cgh-cg > ca then ca=cgh-cg end
	if cbh-cb > ca then ca=cbh-cb end

	return cr,cg,cb,ca
end

M.ramp_is_simp=function(ramp)

	if #ramp ~= 3 then return false end

	local cr,cg,cb,ca = M.ramp_to_simp(ramp)
	
	local temp = M.simp_to_ramp(cr,cg,cb,ca)
	
	if temp[1] ~= ramp[1] then return false end
	if temp[2] ~= ramp[2] then return false end
	if temp[3] ~= ramp[3] then return false end
	
	return true
end

M.random_soul=function(opts)
	opts=opts or {}
	
	local rnd
	if opts.seed then
		rnd=rnd64k(opts.seed)
	else
		rnd=rnd64k(math.random()*65535)
	end

	local soul={}
	
	soul.tweaks={}
	soul.parts={}
	soul.materials={}

	for i,name in ipairs( M.part_names ) do
	
		local chance=1.0
		if name=="hat" or name=="eyewear" or name=="beard" or name=="tail" or name=="item" then
			chance=0.1
		end
		
		local r=rnd()

--print( name, r , chance , r < chance )

		if r < chance then -- object
		
			local poss={}
			
			for _,v in ipairs( M.mesh_names ) do
			
				local s=v[1]
				if s:sub(1,#name+1)==name.."_" then
					if v.group==1 then
						poss[#poss+1]=v[1]
					end
				end
			end
			
			if #poss>0 then
				local i=rnd(1,#poss)
				soul.parts[name]={{name=poss[i],flags=3}}
			else
				soul.parts[name]={}
			end

		else
			soul.parts[name]={}
		end

	end
	
	if soul.parts.hat[1] then soul.parts.hair[1]={name="hair_base",flags=3} end -- smaller hair if we have a hat
	if soul.parts.item[1] then soul.parts.item[1].flags=rnd(1,2) end -- only 1 item in a random hand

	local cmap={
		0x11cccccc,
		0x11888888,
		0x11000000,
		0x11ee9988,
		0x11dd8888,
		0x11884444,
		0x11444488,
		0x11cccccc,
		0x11cc4444,
		0x11cc8844,
		0x11cccc44,
		0x1144cc44,
		0x114444cc,
		0x1144cccc,
		0x11cc8888,
		0x11884444,
	}
	for i,name in ipairs( M.material_names ) do
	
		local r,g,b,a=wpack.argb8_b4(cmap[i])
					
		r=r+rnd(-64,64)
		g=g+rnd(-64,64)
		b=b+rnd(-64,64)
		a=a+rnd(-16,64)
		
		local ramp=M.simp_to_ramp(r,g,b,a)

		soul.materials[name]={ ramp=ramp }
	
	end
	
	for i,name in ipairs( M.tweak_names ) do
		local tweak={}
		soul.tweaks[name]=tweak
		tweak.scale={}
		tweak.scale[1]    =rnd(0.95,1.05)
		tweak.scale[2]    =rnd(0.95,1.05)
		tweak.scale[3]    =rnd(0.95,1.05)
		tweak.rotate={}
		tweak.rotate[1]   =rnd(-3,3)
		tweak.rotate[2]   =rnd(-3,3)
		tweak.rotate[3]   =rnd(-3,3)
		tweak.translate={}
		tweak.translate[1]=rnd(-0.003,0.003)
		tweak.translate[2]=rnd(-0.003,0.003)
		tweak.translate[3]=rnd(-0.003,0.003)

		if name=="eye" or name=="ear" or name=="mouth" or name=="cheek" or name=="boob" then
			tweak.scale[1]    =rnd(0.75,1.25)
			tweak.scale[2]    =rnd(0.75,1.25)
			tweak.scale[3]    =rnd(0.75,1.25)
		end

		if name=="body" or name=="item" or name=="hat" or name=="hair" or name=="eyeball" then
			tweak.scale[1]    =1
			tweak.scale[2]    =1
			tweak.scale[3]    =1
			tweak.rotate[1]   =0
			tweak.rotate[2]   =0
			tweak.rotate[3]   =0
			tweak.translate[1]=0
			tweak.translate[2]=0
			tweak.translate[3]=0
		end
	end
	
	return soul
end

M.bake=function(oven,geoms_avatar)

	local wgeom=oven.rebake("wetgenes.gamecake.spew.geom")
	local wgeom_gltf=oven.rebake("wetgenes.gamecake.spew.geom_gltf")
	local wgeoms=oven.rebake("wetgenes.gamecake.spew.geoms")

	local gl=oven.gl

	local textures=oven.cake and oven.cake.textures
	
	geoms_avatar.bonetexs={} -- animation converted into a shader texture
	

	geoms_avatar=geoms_avatar or {}
	for n,v in pairs(M) do geoms_avatar[n]=v end
	geoms_avatar.modname=M.modname

-- we DO have OpenGL access here

	function geoms_avatar.loads()

		if not io.open(geoms_avatar.filename,"r") then
			geoms_avatar.filename="data/glb/avatar.glb"
		end -- the gc will close the file...

		geoms_avatar.gltf=wgeom_gltf.load(geoms_avatar.filename)
		geoms_avatar.objs=wgeom_gltf.to_geoms(geoms_avatar.gltf)
		
		local map={}
		for _,v in pairs(M.mesh_names) do
			map[ v[1] ]=v
		end
		for _,o in ipairs(geoms_avatar.objs) do -- add parts
			if o.name and not map[o.name] then
--print("adding",o.name)
				local v={o.name,group=0}
				map[o.name]=v
				M.mesh_names[#M.mesh_names+1]=v
			end
		end
		
--[[
		log("avatar",#geoms_avatar.objs.anims)
		for i,v in ipairs(geoms_avatar.objs.anims) do
			log("avatar","anim",i,math.floor(v.min*24),math.floor(v.max*24),v.name)
		end		
]]

		geoms_avatar.build_texture_anims()

if gl then
	gl.program_source("avatar_gltf",[[

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

uniform sampler2D fixbones;
uniform sampler2D texbones;

uniform float animframe;

#ifdef VERTEX_SHADER

 
in vec4  a_color;
in vec3  a_vertex;
in vec3  a_normal;
in vec4  a_tangent;
in vec2  a_texcoord;
in float a_matidx;
in vec4  a_bone;


out vec4  v_color;
out vec3  v_normal;
out vec3  v_tangent;
out vec3  v_bitangent;
out vec3  v_pos;
out vec2  v_texcoord;
out float v_matidx;
out vec4  v_bone;
out vec4  v_value;


#ifdef SHADOW
uniform mat4 camera;
uniform mat4 shadow_mtx;
out vec4  shadow_uv;
#endif

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
		vec4(0.0,0.0,0.0,1.0)) );
}

mat4 getbone(int bidx)
{
	float fb=fract(animframe);
	float fa=1.0-fb;

	mat4 ma=texbone( bidx , int(animframe    ) );
	mat4 mb=texbone( bidx , int(animframe+1.0) );
	mat4 mab=(((fa*ma)+(fb*mb)));

	mat4 mc=fixbone( bidx , 0 );
	mat4 md=fixbone( bidx , 1 );
	mat4 me=fixbone( bidx , 2 );

	mat4 mf=mat4( mat3(me)*mat3(mab) );
	mf[3]=mab[3];
	
	mat4 mr=md*mc*mf;

	
	return mr*mc;

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
	

#ifdef SHADOW
	shadow_uv = ( shadow_mtx * camera * v * vec4(a_vertex,1.0) ) ;
	shadow_uv = vec4( ( shadow_uv.xyz / shadow_uv.w ) * 0.5 + 0.5 ,
		normalize( mat3( shadow_mtx * camera * v ) * a_normal ).z );
#endif


}


#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER

in vec4  v_color;
in vec3  v_normal;
in vec3  v_tangent;
in vec3  v_bitangent;
in vec3  v_pos;
in vec2  v_texcoord;
in float v_matidx;
in vec4  v_bone;
in vec4  v_value;

out vec4 FragColor;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

#ifdef SHADOW
uniform sampler2D shadow_map;
in vec4  shadow_uv;
#endif

void main(void)
{

	vec4 t0=vec4(1.0,1.0,1.0,1.0);
	vec4 t1=vec4(0.5,0.5,0.5,0.5);
	vec3 t2=vec3(0.5,0.5,1.0);
	

	
	
	if( v_value.g > 0.0 || v_value.b > 0.0 )
	{
		t1 = texture(tex1, v_texcoord ).rgba;
	}

	if( v_value.a > 0.0 )
	{
		t2 = v_value.a * texture(tex2, v_texcoord ).rgb;
	}
	
	t2=(t2-vec3(0.5,0.5,0.5))*2.0;

	vec3 n = normalize( (t2.x*v_bitangent) + (-t2.y*v_tangent) + (t2.z*v_normal) );

	if( v_value.r > 0.0 )
	{
		vec2 uv=clamp( v_texcoord + vec2( pow( max( max( n.z, -n.y ) , 0.0 ) , 4.0 )-0.5 ,0.0) , vec2(0.0,0.0) , vec2(1.0,1.0) ) ;

		t0 = v_value.r * texture(tex0, uv ).rgba;
	}

	FragColor=vec4( ( t0 * v_color ).rgb , 1.0 ) ;

#ifdef SHADOW

	const vec4 shadow=vec4(SHADOW);

	if( (shadow_uv.x > 0.0)  && (shadow_uv.x < 1.0) && (shadow_uv.y > 0.0) && (shadow_uv.y < 1.0) )
	{


		float shadow_value = 0.0;
		vec2 shadow_texel_size = 1.0 / vec2( textureSize(shadow_map,0) );
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				shadow_value +=smoothstep(
					-(( 1.0 - abs( shadow_uv.w ) )*shadow[3]+shadow[2]) ,
					-shadow[1] ,
					texture(shadow_map, shadow_uv.xy + vec2(x,y)*shadow_texel_size ).r - shadow_uv.z );
			}    
		}
		shadow_value /= 9.0;

		FragColor=vec4( FragColor.rgb*( shadow_value*shadow[0] + (1.0-shadow[0]) ) , FragColor.a );
	}

#endif

}


#endif //FRAGMENT_SHADER

]])
end
	end
	
	function geoms_avatar.build_bone_masks(avatar)
	
		local masks={}
		avatar.bone_masks=masks
		
		masks[0]={} -- empty
		masks[1]={} -- left
		masks[2]={} -- right
		masks[3]={} -- left+right
	
		local objs=geoms_avatar.objs
		local skin=objs.skins[1] -- assume only one boned character
		if skin then
			local b=0
			for i,v in ipairs(skin.nodes) do
				local bb,bl,br=false,false,false
				if v.name:sub(-2,-1)==".L" then bl=true bb=true end
				if v.name:sub(-2,-1)==".R" then br=true bb=true end
				
				if bb then -- a left/right bone
					if bl then
						if br then
							masks[0][i]=false
							masks[1][i]=true
							masks[2][i]=true
							masks[3][i]=true
						else
							masks[0][i]=false
							masks[1][i]=true
							masks[2][i]=false
							masks[3][i]=true
						end
					else
						if br then
							masks[0][i]=false
							masks[1][i]=false
							masks[2][i]=true
							masks[3][i]=true
						else
							masks[0][i]=false
							masks[1][i]=false
							masks[2][i]=false
							masks[3][i]=false
						end
					end
				else
					masks[0][i]=true
					masks[1][i]=true
					masks[2][i]=true
					masks[3][i]=true
				end
			end
		end

	end


	function geoms_avatar.build_texture_tweak(avatar)
	
		local width=0
		local height=3
		
		local fs={}
		
		local ts=wgeoms.get_anim_tweaks(geoms_avatar.objs)
		width=(#ts/(4*height))

-- upload breaks on webgl?
-- so we just delete if it already exists
		if avatar.fixtex then
			textures.delete(avatar.fixtex)
			avatar.fixtex=nil
		end

		
--			avatar.fixtex.gl_table=ts
--			avatar.fixtex.gl_data=wpack.save_array(avatar.fixtex.gl_table,"f32")
--			avatar.fixtex:upload()

--		else
if gl then
			avatar.fixtex=textures.create({
				gl_data=wpack.save_array(ts,"f32"),
				gl_table=ts,
				gl_width=width,
				gl_height=height,
				gl_internal=gl.RGBA32F,
				gl_format=gl.RGBA,
				gl_type=gl.FLOAT,
			})
end
--		end

	end

	function geoms_avatar.adjust_texture_tweak(avatar,name,mat)

		local skin=geoms_avatar.objs.skins[1] -- assume only one boned character
		if not skin then return end
		if not avatar.fixtex then return end
		
		local bones=avatar.fixtex.gl_table
	

		local bs=#skin.nodes
		local save=function(bone,b)

			bones[b+1]=bone[1]
			bones[b+2]=bone[5]
			bones[b+3]=bone[9]
			bones[b+4]=bone[13]

			bones[b+5]=bone[2]
			bones[b+6]=bone[6]
			bones[b+7]=bone[10]
			bones[b+8]=bone[14]

			bones[b+9]=bone[3]
			bones[b+10]=bone[7]
			bones[b+11]=bone[11]
			bones[b+12]=bone[15]
		end

		for i,v in ipairs(skin.nodes) do
		
			if v.name==name then
				save( mat , ((bs*2)+i-1)*12 )
			end

		end

		avatar.fixtex.gl_table=bones
		avatar.fixtex.gl_data=wpack.save_array(avatar.fixtex.gl_table,"f32")
		avatar.fixtex:upload()

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
			
--			log("avatar",anim.name,#fs)
			
			if textures then
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
		
	end

	function geoms_avatar.update_pose(avatar)
		local objs=geoms_avatar.objs
		avatar.anim=objs.anims[avatar.pose] or objs.anims[1]
		if avatar.anim then
			avatar.time=(avatar.time or 0)%1
		end
	end
	
	function geoms_avatar.update(avatar)

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

		avatar.bones=bones
		avatar.material_colors=material_colors
		avatar.material_values=material_values


		avatar.anim=objs.anims[avatar.pose] or objs.anims[1]
		if avatar.anim then
			if avatar.anim.length and avatar.anim.length>0 then
				avatar.time=(avatar.time or 0)+(avatar.speed/avatar.anim.length)
			end
			avatar.time=(avatar.time or 0)%1
		end
		wgeoms.update(objs)

	end

	function geoms_avatar.draw(avatar,opts)

		opts=opts or {}

		local pp=function(p)
		
			gl.ActiveTexture(gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			avatar.fixtex:bind()
			gl.Uniform1i( p:uniform("fixbones"), gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

			gl.ActiveTexture(gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			geoms_avatar.bonetexs[avatar.anim.name]:bind()
			gl.Uniform1i( p:uniform("texbones"), gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

			gl.ActiveTexture(gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			oven.cake.images.bind(avatar.image)
			gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

			gl.ActiveTexture( gl.TEXTURE0 )

			local objs=geoms_avatar.objs
			if avatar.anim then
				local anim=avatar.anim
				anim.length=anim.max-anim.min
				avatar.time=(avatar.time or 0)%1
				if anim.keys[2] then
					local rate=anim.keys[2]-anim.keys[1]
					avatar.frame=(avatar.time*anim.length/rate)
				else
					avatar.frame=0
				end
				gl.Uniform1f( p:uniform("animframe"), avatar.frame )
			end

			if avatar.bones then
				gl.UniformMatrix4f( p:uniform("bones"), avatar.bones )
				gl.Uniform4f( p:uniform("material_colors"), avatar.material_colors )
				gl.Uniform4f( p:uniform("material_values"), avatar.material_values )
			end
			
		end


		local shadername=opts.shadername or "gamecake_shader?CAM&NORMAL&TEX&TEXBONE=80&TEXNTOON=1"

		if opts.shadow then

			wgeom.draw(avatar.obj,shadername.."&SHADOW="..opts.shadow ,pp)

		else

			wgeom.draw(avatar.obj,shadername,pp)

		end
		
-- draw debug bones	
--		wgeoms.draw_bones(geoms_avatar.objs,"avatar_gltf",pp)

	end
	

	function geoms_avatar.rebuild(avatar,soul)

		local clamp=function(a) if a<0 then a=0 elseif a>255 then a=255 end return math.floor(a) end
		local rgb=function(r,g,b)
			return          255  * 256*256*256 + 
					clamp(r*255) * 256*256 + 
					clamp(g*255) * 256 + 
					clamp(b*255)
		end
		avatar.map:clear(0xffffffff)
		for idx,name in ipairs(geoms_avatar.material_names) do
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
				local g=avatar.map:clip( 0,idx-1,0, 64,1,1 )
				wgrdcanvas.cmap_ramp(g,keys)
			end
		end
		if gl then
			oven.cake.images.unload(avatar.image) -- force a reload
		end


		local obj=wgeom.new()
		local show={}		
		for i,v in pairs(soul.parts) do
			for i,p in ipairs(v) do
				if p.name then
					show[p.name]=p.flags
				end
			end
		end
		
		for _,o in ipairs(geoms_avatar.objs) do
			if show[ o.name ] then
				local t=o:duplicate():filter_by_bones( avatar.bone_masks[ show[ o.name ] ] or avatar.bone_masks[0] )
				obj:merge_from(t)
			end
		end
		avatar.obj=obj

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

-- half the tweak as we will apply it twice in the shader

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

						node.tweak[ 8]=((tweak.scale[1]-1))+1
						node.tweak[ 9]=((tweak.scale[2]-1))+1
						node.tweak[10]=((tweak.scale[3]-1))+1
				end
			end
		end

		geoms_avatar.build_texture_tweak(avatar)

	end

geoms_avatar.simp_to_ramp=M.simp_to_ramp
geoms_avatar.ramp_to_simp=M.ramp_to_simp
geoms_avatar.ramp_is_simp=M.ramp_is_simp
geoms_avatar.random_soul=M.random_soul

	function geoms_avatar.avatar(avatar,soul)
	
		avatar=avatar or {}
		
		avatar.adjust_texture_tweak=function(name,mat)
			geoms_avatar.adjust_texture_tweak(avatar,name,mat)
		end

		avatar.rebuild=function( soul )
			if type(soul)=="string" then
				soul=geoms_avatar.random_soul({seed=soul})
			end
			avatar.soul=soul
			geoms_avatar.rebuild( avatar , soul )
		end

		avatar.setup=function( soul )

			avatar.speed=1/60

			soul=soul or geoms_avatar.random_soul({})

			avatar.anim=geoms_avatar.objs.anims[1]

			avatar.map=wgrd.create(wgrd.FMT_U8_RGBA_PREMULT,64,16,1)
			avatar.map:clear(0xffffffff)
			
			if gl then
				avatar.image=oven.cake.images.load("avatar/"..tostring(avatar),"avatar/"..tostring(avatar),function() return avatar.map end)
				avatar.image.TEXTURE_WRAP_S		=	gl.CLAMP_TO_EDGE
				avatar.image.TEXTURE_WRAP_T		=	gl.CLAMP_TO_EDGE
				avatar.image.TEXTURE_MIN_FILTER	=	gl.LINEAR
				avatar.image.TEXTURE_MAX_FILTER	=	gl.LINEAR
			end
			
			geoms_avatar.build_texture_tweak(avatar)
			geoms_avatar.build_bone_masks(avatar)

			avatar.rebuild( soul )
		end
				
		avatar.update_pose=function()
			geoms_avatar.update_pose(avatar)
		end
	
		avatar.update=function()
			geoms_avatar.update(avatar)
		end

		avatar.draw=function(opts)
			geoms_avatar.draw(avatar,opts)
		end

		avatar.setup(soul)
	
		return avatar
	end
	

	return geoms_avatar
end


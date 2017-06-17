--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- this contains mild emulation of the old gl fixed pipeline, such that we can run some code in gles2 and above

local tardis=require("wetgenes.tardis")
local wzips=require("wetgenes.zips")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,gles)

	if not oven.gl then -- need a gles2 wrapped in glescode
	
		oven.gl=require("glescode").create( require("gles").gles2 )
		
		oven.gl.GetExtensions()
		
	end

	local gl=oven.gl
	
	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

--[==[
	
-- these are default simple shaders
--
-- if the v shader had no normal then we assume "special z mode"
-- where the z component is treated as 0 for view transform
-- but added to z in viewspace
--
-- this allows very simple depth buffer sorting of 2d polys
--
-- if we have normals then the transform is applied to full xyz
-- and the normal which is then noralized for lighting calcs

	gl.shaders.v_pos_tex={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex.xy, 0.0 , 1.0);
    gl_Position.z+=a_vertex.z;
	v_texcoord=a_texcoord;
	v_color=color;
}

	]]
}

	gl.shaders.v_xyz_tex={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex.xyz , 1.0);
	v_texcoord=a_texcoord;
	v_color=color;
}

	]]
}


	gl.shaders.v_pos_tex_color={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;
attribute vec4 a_color;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex.xy, 0.0 , 1.0);
    gl_Position.z+=a_vertex.z;
	v_texcoord=a_texcoord;
	v_color=a_color*color;
}

	]]
}

	gl.shaders.v_raw_tex_color={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;
attribute vec4 a_color;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * vec4(a_vertex.xyz , 1.0);
	v_texcoord=a_texcoord;
	v_color=a_color;
}

	]]
}

	gl.shaders.v_raw_tex={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * vec4(a_vertex.xyz , 1.0);
	v_texcoord=a_texcoord;
	v_color=color;
}

	]]
}

	gl.shaders.v_xyz={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;

varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex.xyz , 1.0);
	v_color=color;
}

	]]
}

	gl.shaders.v_pos={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;

varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex.xy, 0.0 , 1.0);
    gl_Position.z+=a_vertex.z;
	v_color=color;
}

	]]
}


	gl.shaders.v_pos_color={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec4 a_color;

varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex.xy, 0.0 , 1.0);
    gl_Position.z+=a_vertex.z;
	v_color=a_color*color;
}

	]]
}

	gl.shaders.f_tex={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform sampler2D tex;

varying vec2  v_texcoord;
varying vec4  v_color;

void main(void)
{
	if( v_texcoord[0] <= -1.0 ) // special uv request to ignore the texture (use -2 as flag)
	{
		gl_FragColor=v_color ;
	}
	else
	{
		gl_FragColor=texture2D(tex, v_texcoord) * v_color ;
	}
}

	]]
}
	gl.shaders.f_tex_discard={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform sampler2D tex;

varying vec2  v_texcoord;
varying vec4  v_color;

void main(void)
{
	if( v_texcoord[0] <= -1.0 ) // special uv request to ignore the texture (use -2 as flag)
	{
		gl_FragColor=v_color ;
	}
	else
	{
		gl_FragColor=texture2D(tex, v_texcoord) * v_color ;
	}
	if((gl_FragColor.a)<0.25) discard;
}

	]]
}

	gl.shaders.f_color={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


varying vec4  v_color;

void main(void)
{
	gl_FragColor=v_color ;
}

	]]
}
	gl.shaders.f_color_discard={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


varying vec4  v_color;

void main(void)
{
	gl_FragColor=v_color ;
	if((gl_FragColor.a)<0.25) discard;
}

	]]
}
	gl.shaders.f_color_mask={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


varying vec4  v_color;
varying float v_matidx;

void main(void)
{
	gl_FragColor=v_color ;
	gl_FragColor.a*=v_matidx;
	if((gl_FragColor.a)<0.25) discard;
}

	]]
}
	gl.shaders.v_pos_normal={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec3 a_normal;

varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex, 1.0);
    v_normal = normalize( mat3( modelview ) * a_normal );
	v_color=color;
}

	]]
}
	gl.shaders.v_xyz_normal=gl.shaders.v_pos_normal

	gl.shaders.v_pos_normal_tex={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


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
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex, 1.0);
    v_normal = normalize( mat3( modelview ) * a_normal );
	v_texcoord=a_texcoord;
	v_color=color;
}

	]]
}
	gl.shaders.v_xyz_normal_tex=gl.shaders.v_pos_normal_tex

	gl.shaders.v_pos_normal_tex_mat={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3  a_vertex;
attribute vec3  a_normal;
attribute vec2  a_texcoord;
attribute float a_matidx;

varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;
varying vec2  v_texcoord;
varying float v_matidx;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex, 1.0);
    v_normal = normalize( mat3( modelview ) * a_normal );
	v_texcoord=a_texcoord;
	v_color=color;
	v_matidx=a_matidx;
}

	]]
}
	gl.shaders.v_xyz_normal_tex_mat=gl.shaders.v_pos_normal_tex_mat

	gl.shaders.v_pos_normal_tex_mat_bone={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

uniform vec4 bones[64*3]; // 64 bones
uniform vec4 bone_fix; // min,max,0,0 (bone ids stored in bones array)

attribute vec3  a_vertex;
attribute vec3  a_normal;
attribute vec2  a_texcoord;
attribute float a_matidx;
attribute vec4  a_bone;

varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;
varying vec2  v_texcoord;
varying float v_matidx;
 
void main()
{
	mat4 m=mat4(0.0);
	vec4 v=vec4(a_vertex, 1.0);
	vec3 n=vec3(a_normal);
	int b;
	if( a_bone[0] > 0.0 ) // got bones
	{
		for(b=0;b<4;b++)
		{
			int i=int(a_bone[b])*3;
			if(i>=3)
			{
				m+=mat4(bones[i-3],bones[i-2],bones[i-1],vec4(0.0,0.0,0.0,1.0))
					*(1.0-fract(a_bone[b]));
			}
			else { break; }
		}
		v=v*m;
		n=n*mat3(m);
	}
	
	gl_PointSize=3.0;
    gl_Position = projection * modelview * v;
    v_normal = normalize( mat3( modelview ) * n );
	v_texcoord=a_texcoord;
	v_color=color;
	v_matidx=a_matidx;
}

	]]
}
	gl.shaders.v_xyz_normal_tex_mat_bone=gl.shaders.v_pos_normal_tex_mat_bone

	gl.shaders.f_phong={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;

void main(void)
{
	vec3 n=normalize(v_normal);
	gl_FragColor= vec4(v_color.rgb*max( n.z, 0.25 ),v_color.a);
}

	]]
}

	gl.shaders.f_phong_light={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;

uniform mat4 modelview;

uniform vec3  light_normal;
uniform vec4  light_color;

void main(void)
{
	vec3 n=normalize( v_normal );
	vec3 l=normalize( mat3( modelview ) * light_normal );
	
	gl_FragColor=vec4(
		v_color.rgb*light_color.rgb * max( dot(l,n),
		light_color.a ),v_color.a);
}

	]]
}

	gl.shaders.f_phong_mat={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


varying vec4  v_color;
varying vec3  v_normal;
varying vec3  v_pos;
varying float v_matidx;


uniform vec4 colors[2*16]; // 16 materials

vec3 d=vec3(0,0,1);

void main(void)
{
	vec3 n=normalize(v_normal);
	vec3 l=normalize(vec3(0.0,-0.5,1.0));

	int matidx=int(floor(v_matidx+0.5));
	vec4 c1=colors[0+matidx*2];
	vec4 c2=colors[1+matidx*2];

	gl_FragColor= vec4(  c1.rgb *      max( n.z      , 0.25 ) + 
						(c2.rgb * pow( max( dot(n,l) , 0.0  ) , c2.a )).rgb , c1.a );
}

	]]
}

	gl.shaders.v_xyz={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;

varying vec4  v_color;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex , 1.0);
	v_color=color;
}

	]]
}
	gl.shaders.v_xyz_mask={
	source=[[{shaderprefix}
#line ]]..debug.getinfo(1).currentline..[[


uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute float a_matidx;

varying vec4  v_color;
varying float v_matidx;
 
void main()
{
	gl_PointSize=3.0;
    gl_Position = projection * modelview * vec4(a_vertex , 1.0);
	v_color=color;
	v_matidx=a_matidx;
}

	]]
}

	gl.programs.pos_normal={
		vshaders={"v_pos_normal"},
		fshaders={"f_phong"},
	}
	gl.programs.pos_normal_light={
		vshaders={"v_pos_normal"},
		fshaders={"f_phong_light"},
	}
	gl.programs.pos_color={
		vshaders={"v_pos_color"},
		fshaders={"f_color"},
	}
	gl.programs.pos_color_discard={
		vshaders={"v_pos_color"},
		fshaders={"f_color_discard"},
	}
	gl.programs.xyz_tex={
		vshaders={"v_xyz_tex"},
		fshaders={"f_tex"},
	}
	gl.programs.pos_tex={
		vshaders={"v_pos_tex"},
		fshaders={"f_tex"},
	}
	gl.programs.xyz_tex_discard={
		vshaders={"v_xyz_tex"},
		fshaders={"f_tex_discard"},
	}
	gl.programs.pos_tex_discard={
		vshaders={"v_pos_tex"},
		fshaders={"f_tex_discard"},
	}
	gl.programs.pos_tex_color={
		vshaders={"v_pos_tex_color"},
		fshaders={"f_tex"},
	}
	gl.programs.pos_tex_color_discard={
		vshaders={"v_pos_tex_color"},
		fshaders={"f_tex_discard"},
	}
	gl.programs.raw_tex_color={
		vshaders={"v_raw_tex_color"},
		fshaders={"f_tex"},
	}
	gl.programs.raw_tex_color_discard={
		vshaders={"v_raw_tex_color"},
		fshaders={"f_tex_discard"},
	}
	gl.programs.raw_tex={
		vshaders={"v_raw_tex"},
		fshaders={"f_tex"},
	}
	gl.programs.raw_tex_discard={
		vshaders={"v_raw_tex"},
		fshaders={"f_tex_discard"},
	}
	gl.programs.xyz={
		vshaders={"v_xyz"},
		fshaders={"f_color"},
	}
	gl.programs.pos={
		vshaders={"v_pos"},
		fshaders={"f_color"},
	}
	gl.programs.pos_discard={
		vshaders={"v_pos"},
		fshaders={"f_color_discard"},
	}

	gl.programs.xyz={
		vshaders={"v_xyz"},
		fshaders={"f_color"},
	}
	gl.programs.xyz_mask={
		vshaders={"v_xyz_mask"},
		fshaders={"f_color_mask"},
	}
	gl.programs.xyz_normal={
		vshaders={"v_xyz_normal"},
		fshaders={"f_phong"},
	}	
	gl.programs.xyz_normal_light={
		vshaders={"v_xyz_normal"},
		fshaders={"f_phong_light"},
	}	

	gl.programs.xyz_normal_mat={
		vshaders={"v_xyz_normal_tex_mat"},
		fshaders={"f_phong_mat"},
	}	

	gl.programs.xyz_normal_mat_bone={
		vshaders={"v_xyz_normal_tex_mat_bone"},
		fshaders={"f_phong_mat"},
	}	

	-- we have mostly squirted extra stuff into oven.gl
]==]

	return gles

end

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,wetiso)
	local wetiso=wetiso or {}
	wetiso.oven=oven
	
	wetiso.modname=M.modname

	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=oven.gl
	local sheets=cake.sheets
	local fbs=cake.framebuffers

	local geom=oven.rebake("wetgenes.gamecake.spew.geom")


wetiso.loads=function()

end
		
wetiso.setup=function()
	if wetiso.setup_done then return end
	wetiso.setup_done=true
	
	wetiso.loads()

	wetiso.it=geom.icosahedron()
	local swap={
			{	3+16	,	1	,	0	},
			{	7		,	2	,	2	},
			{	4+16	,	3	,	1	},
			{	4+16	,	4	,	0	},
			{	4+16	,	5	,	1	},
			{	5+8		,	6	,	0	},
			{	7		,	7	,	2	},
			{	6+8		,	8	,	1	},

			{	2+16	,	1+8	,	0	},
			{	6+8		,	2+8	,	2	},
			{	1+16	,	3+8	,	1	},
			{	4+16	,	4+8	,	0	},
			{	5+8		,	5+8	,	1	},
			{	8+8		,	6+8	,	0	},
			{	3+16	,	7+8	,	2	},
			{	3+16	,	8+8	,	1	},

			{	1+16	,	1+16	,	0	},
			{	4+16	,	2+16	,	1	},
			{	4+16	,	3+16	,	0	},
			{	4+16	,	4+16	,	2	},

		}
	local n=wetiso.it.polys
	for i,v in pairs(swap) do
		n[ v[1] ] , n[ v[2] ] = n[ v[2] ] , n[ v[1] ]
		
		local p=n[ v[2] ]
		for i=1,v[3] do
			table.insert(p,1,table.remove(p,#p)) -- rotate
		end
	end

	geom.apply_bevel(wetiso.it,7/8)

	wetiso.fbo=fbs.create()
--	wetiso.fbo.TEXTURE_MIN_FILTER=gl.NEAREST
--	wetiso.fbo.TEXTURE_MAX_FILTER=gl.NEAREST

--	wetiso.predraw()
end

wetiso.clean=function()

	wetiso.fbo:clean()
	wetiso.fbo=nil

	wetiso.setup_done=false
	
end

wetiso.predraw=function()

	if not gl.programs.geom_wetiso then -- setup our special shaders
	
		gl.shaders.v_geom_wetiso={
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

		gl.shaders.f_geom_wetiso={
		source=[[

uniform sampler2D tex;

varying vec4  v_color;
varying vec4  v_color2;
varying vec3  v_normal;
varying vec3  v_pos;
varying vec2  v_texcoord;
varying float v_face;


vec3 d=vec3(0.0,0.0,-1.0);

void main(void)
{
	vec4 tc=vec4(0.0,0.0,0.0,0.0);
	if( floor(v_face+(1.0/4096.0)) == ceil(v_face-(1.0/4096.0)) )
	{
		tc=texture2D(tex, v_texcoord);
	}

	vec3 n=normalize(v_normal);
	gl_FragColor= tc + (v_color*max( n.z, 0.25 ))*(1.0-tc.a) ;
	gl_FragColor.a=1.0;
}

	]]
}

		gl.programs.geom_wetiso={
			vshaders={"v_geom_wetiso"},
			fshaders={"f_geom_wetiso"},
		}
	
	end

	local f1=cake.fonts.get("Vera")
	local f2=f1.images[1]
	if not wetiso.fbo.texture or wetiso.font_mip~=f2.gl_mip then -- build our texture (happens after any stop/start)
		wetiso.font_mip=f2.gl_mip
		wetiso.fbo:clean()

		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()

		wetiso.fbo:resize(1024,128,0)		
		wetiso.fbo:bind_frame()
		local layout=cake.layouts.create{parent={w=wetiso.fbo.w,h=wetiso.fbo.h,x=0,y=0}}
		local oldlay=layout.apply(wetiso.fbo.w,wetiso.fbo.h,1/4,wetiso.fbo.h*4)
		
		gl.ClearColor(pack.argb4_pmf4(0x0000))
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		local font_cache_draw = font.cache_begin()

--		cells.draw_walls()
			font.set(cake.fonts.get("Vera")) -- default font
			font.set_size(64) -- 32 pixels high

			gl.Color(1,1,1,1)
			for i=1,8 do
				local a=string.sub("W tgenes",i,i)
--				local a=string.sub("12345678",i,i)
				local w=font.width(a)
				font.set_xy((i-1)*128 + (128-w)/2 ,48)
				font.draw(a)
			end
--			font.draw("abcdefghijklmnopqrstuvwxyz")

			local a="e" -- need an upside down E
			local w=font.width(a)
			gl.Translate(128 + 64 ,64)
			gl.Rotate(180 ,0,0,1 )
			font.set_xy( -(w/2)  , -32 -32 )
			font.draw(a)
			
		font_cache_draw()


		fbs.bind_frame(nil)
		
		wetiso.fbo:free_depth()
		wetiso.fbo:free_frame()
		-- but keep the texture and update its mipmaps
		wetiso.fbo:mipmap()

		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()
		
--		main.layout.restore()
		oldlay.restore()

	end

end

wetiso.draw=function()

 	wetiso.predraw() -- make sure buffers are valid
		
	wetiso.fbo:bind_texture()
	geom.draw_polys(wetiso.it,"geom_wetiso")
	
end

	return wetiso
end

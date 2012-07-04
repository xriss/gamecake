-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- this contains mild emulation of the old gl fixed pipeline, such that we can run some code in gles2 and above

local tardis=require("wetgenes.tardis")

local glescode=require("glescode")

local glesfix={}


local shaderprefix="#version 100\nprecision mediump float;\n"


--shaderprefix=="#version 120\n"

-- apply our compatibility fixes into the base gles function table
-- return a wrapped table with new functionality
function glesfix.create(gles)

	local gl=glescode.create(gles)
	gl.fix={}
	gl.fix.client={}
	gl.fix.pointer={}


	gl.shaders.v_pos_tex={
	source=shaderprefix..[[

uniform mat4 modelview;
uniform mat4 projection;

attribute vec3 vertex;
attribute vec2 texcoord;
uniform vec4 color;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4(vertex, 1.0);
	v_texcoord=texcoord;
	v_color=color;
}

	]]
}

	gl.shaders.v_pos={
	source=shaderprefix..[[

uniform mat4 modelview;
uniform mat4 projection;

attribute vec3 vertex;
uniform vec4 color;

varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4(vertex, 1.0);
	v_color=color;
}

	]]
}

	gl.shaders.f_pos_tex={
	source=shaderprefix..[[

uniform sampler2D tex;

varying vec2  v_texcoord;
varying vec4  v_color;

void main(void)
{
	gl_FragColor=texture2D(tex, v_texcoord) * v_color ;
}

	]]
}

	gl.shaders.f_pos={
	source=shaderprefix..[[

varying vec4  v_color;

void main(void)
{
	gl_FragColor=v_color ;
}

	]]
}

	gl.programs.pos_tex={
		vshaders={"v_pos_tex"},
		fshaders={"f_pos_tex"},
	}
	gl.programs.pos={
		vshaders={"v_pos"},
		fshaders={"f_pos"},
	}

	function gl.Color(...)
		gl.fix.color={...}
	end

	function gl.EnableClientState(n)
		gl.fix.client[n]=true
	end
	function gl.DisableClientState(n)
		gl.fix.client[n]=false
	end

	function gl.VertexPointer(...)
		gl.fix.pointer.vertex={...}
	end
	function gl.TexCoordPointer(...)
		gl.fix.pointer.texcoord={...}
	end

	function gl.DrawArrays(...) -- make sure state is set before doing
	
		local p
		
		if gl.fix.client[gl.TEXTURE_COORD_ARRAY] then
		
			p=gl.program("pos_tex")
			
		else
		
			p=gl.program("pos")
			
		end
		gl.UseProgram( p[0] )

		
		local v=gl.fix.pointer.vertex
		gl.VertexAttribPointer(p:attrib("vertex"),v[1],v[2],gl.FALSE,v[3],v[4])
		gl.EnableVertexAttribArray(p:attrib("vertex"))
		
		if gl.TEXTURE_COORD_ARRAY then
		
			local v=gl.fix.pointer.texcoord
			gl.VertexAttribPointer(p:attrib("texcoord"),v[1],v[2],gl.FALSE,v[3],v[4])
			gl.EnableVertexAttribArray(p:attrib("texcoord"))
			
		end

--		gl.VertexAttrib4f(p:attrib("color"), 1,1,1,1 )

--		gl.Uniform1i(p:uniform("tex"), 0 )

		gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
		gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )

		gl.Uniform4f(p:uniform("color"), gl.fix.color[1],gl.fix.color[2],gl.fix.color[3],gl.fix.color[4] )

		gl.core.DrawArrays(...)
	end

	return gl
end

return glesfix

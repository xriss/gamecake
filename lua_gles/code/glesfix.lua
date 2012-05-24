-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- this contains mild emulation of the old gl fixed pipeline, such that we can run some code in gles2 and above

local tardis=require("wetgenes.tardis")

local glesfix={}





-- apply our compatibility fixes into the base gles function table
function glesfix.apply_compat(gles)

	gles.fix={}
	gles.fix.client={}
	gles.fix.pointer={}

	gles.stacks={}
	gles.stacks[gles.MODELVIEW]={ tardis.m4.new() }
	gles.stacks[gles.PROJECTION]={ tardis.m4.new() }
	gles.stacks[gles.TEXTURE]={ tardis.m4.new() }
	
	gles.stack_mode=gles.MODELVIEW
	gles.stack=gles.stacks[gles.stack_mode]
	gles.stack_matrix=gles.stack[#gles.stack]
	
	function getmatrix(i)
		local v=gles.stacks[i]
		return v[#v]
	end
	
	gles.shaders={}
	gles.programs={}
	
	gles.shaders.v_pos_tex={
	source=[[
#version 120

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
	gles.shaders.f_pos_tex={
	source=[[
#version 120

uniform sampler2D tex;

varying vec2  v_texcoord;
varying vec4  v_color;

//uniform vec4 color;

void main(void)
{
	gl_FragColor=texture2D(tex, v_texcoord) * v_color ;
}

	]]
}

	gles.programs.pos_tex={
		vshaders={"v_pos_tex"},
		fshaders={"f_pos_tex"},
	}
	
	function shader(stype,sname)

		local s=gles.shaders[sname]
		
		if s[0] then return s[0] end

print("Compiling shader "..sname)
		s[0]=gles.CreateShader(stype)
		gles.ShaderSource(s[0],s.source)
		gles.CompileShader(s[0])
		
		if gles.GetShader(s[0], gles.COMPILE_STATUS) == gles.FALSE then -- error

			print( gles.GetShaderInfoLog(s[0])  , "\n" )

			error( "failed to build shader "..sname )
		end
	
		return s[0]
	end
	
	local pbase={}
	local pmeta={__index=pbase}
	
	function prog(pname)
		local p=assert(gles.programs[pname])
		if not p[0] then
			setmetatable(p,pmeta)

			p.vars={}
			p[0]=gles.CreateProgram()
			
			for i,v in ipairs(p.vshaders) do
				gles.AttachShader( p[0] , shader(gles.VERTEX_SHADER,v) )
			end
			for i,v in ipairs(p.fshaders) do
				gles.AttachShader( p[0] , shader(gles.FRAGMENT_SHADER,v) )
			end
			
print("Linking program "..pname)
			gles.LinkProgram(p[0])
		
			if gles.GetProgram(p[0], gles.LINK_STATUS) == gles.FALSE then -- error

				print( gles.GetProgramInfoLog(p[0]) , "\n" )

				error( "failed to build program "..pname )
			end
			
		end
		return p
	end
	
	function pbase.attrib(p,vname)
		local r=p.vars[vname]
		if r then return r end
		r=gles.GetAttribLocation(p[0],vname)
		p.vars[vname]=r
		return r
	end

	function pbase.uniform(p,vname)
		local r=p.vars[vname]
		if r then return r end
		r=gles.GetUniformLocation(p[0],vname)
		p.vars[vname]=r
		return r
	end
	
	
	function gles.MatrixMode(mode)
		gles.stack_mode=mode
		gles.stack=assert(gles.stacks[gles.stack_mode])
		gles.stack_matrix=assert(gles.stack[#gles.stack])
	end

	function gles.LoadMatrix(...)
		gles.stack_matrix:set(...)
	end

	function gles.MultMatrix(a)
		tardis.m4_product_m4(gles.stack_matrix,a,gles.stack_matrix)
	end

	function gles.Frustum(...)
		error("frustrum not suported")
	end

	function gles.LoadIdentity()
		gles.stack_matrix:identity()
	end

	function gles.Translate(vx,vy,vz)
		gles.stack_matrix:translate({vx,vy,vz})
	end

	function gles.Rotate(d,vx,vy,vz)
		gles.stack_matrix:rotate(d,{vx,vy,vz})
	end

	function gles.Scale(vx,vy,vz)
		gles.stack_matrix:scale_v3({vx,vy,vz})
	end

	function gles.PushMatrix()		
		gles.stack[#gles.stack+1]=tardis.m4.new(gles.stack_matrix) -- duplicate to new top
		gles.stack_matrix=assert(gles.stack[#gles.stack])
	end

	function gles.PopMatrix()
		gles.stack[#gles.stack]=nil -- remove topmost
		gles.stack_matrix=assert(gles.stack[#gles.stack]) -- this will assert on too many pops
	end

	function gles.Color(...)
		gles.fix.color={...}
	end

	function gles.EnableClientState(n)
		gles.fix.client[n]=true
	end
	function gles.DisableClientState(n)
		gles.fix.client[n]=false
	end

	function gles.VertexPointer(...)
		gles.fix.pointer.vertex={...}
	end
	function gles.TexCoordPointer(...)
		gles.fix.pointer.texcoord={...}
	end

	function gles.DrawArrays(...) -- make sure state is set before doing
	
		local p=prog("pos_tex")
		gles.UseProgram( p[0] )

		
		gles.EnableVertexAttribArray(p:attrib("vertex"))
		gles.EnableVertexAttribArray(p:attrib("texcoord"))

		local v=gles.fix.pointer.vertex
		gles.VertexAttribPointer(p:attrib("vertex"),v[1],v[2],gles.FALSE,v[3],v[4])
		
		local v=gles.fix.pointer.texcoord
		gles.VertexAttribPointer(p:attrib("texcoord"),v[1],v[2],gles.FALSE,v[3],v[4])

--		gles.VertexAttrib4f(p:attrib("color"), 1,1,1,1 )

--		gles.Uniform1i(p:uniform("tex"), 0 )

		gles.UniformMatrix4f(p:uniform("modelview"), getmatrix(gles.MODELVIEW) )
		gles.UniformMatrix4f(p:uniform("projection"), getmatrix(gles.PROJECTION) )

		gles.Uniform4f(p:uniform("color"), gles.fix.color[1],gles.fix.color[2],gles.fix.color[3],gles.fix.color[4] )

		gles.core.DrawArrays(...)
	end



end

return glesfix

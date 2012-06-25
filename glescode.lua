-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generate a place to compile gl code into
-- this also contains matrix manipulation functions in the style of normal gl

local tardis=require("wetgenes.tardis")

local glescode={}



-- please pass in the gles base you wish to use, eg gles.gles2
-- returns a state table with compiler functions and places to
-- keep your source.
function glescode.create(gl)

	local code={}

-- matrix functions

	function reset_stacks()
		code.stacks={}		
		code.stack_mode=nil
		code.stack=nil
		code.stack_matrix=nil
	end
	reset_stacks() -- setup

	function code.matrix(mode)
		local v=assert(code.stacks[mode])
		return v[#v]
	end
	
	function code.MatrixMode(mode)
		code.stack_mode=mode
		code.stack=code.stacks[code.stack_mode]
		if not code.stack then -- create on use
			code.stack={ tardis.m4.new() }
			code.stacks[code.stack_mode]=code.stack
		end
		code.stack_matrix=assert(code.stack[#code.stack])
	end

	function code.LoadMatrix(...)
		code.stack_matrix:set(...)
	end

	function code.MultMatrix(a)
		tardis.m4_product_m4(code.stack_matrix,a,code.stack_matrix)
	end

	function code.Frustum(...)
		error("frustrum not suported")
	end

	function code.LoadIdentity()
		code.stack_matrix:identity()
	end

	function code.Translate(vx,vy,vz)
		code.stack_matrix:translate({vx,vy,vz})
	end

	function code.Rotate(d,vx,vy,vz)
		code.stack_matrix:rotate(d,{vx,vy,vz})
	end

	function code.Scale(vx,vy,vz)
		code.stack_matrix:scale_v3({vx,vy,vz})
	end

	function code.PushMatrix()		
		code.stack[#code.stack+1]=tardis.m4.new(code.stack_matrix) -- duplicate to new top
		code.stack_matrix=assert(code.stack[#code.stack])
	end

	function code.PopMatrix()
		code.stack[#code.stack]=nil -- remove topmost
		code.stack_matrix=assert(code.stack[#code.stack]) -- this will assert on too many pops
	end

-- compiler functions

	code.shaders={}
	code.programs={}
		
-- forget cached info when we lose context, it is important to call this
	function code.forget()
		for i,v in pairs(code.shaders) do
			v[0]=nil
		end
		for i,v in pairs(code.programs) do
			v[0]=nil
		end
	end
	
	function code.shader(stype,sname)

		local s
		
		if type(sname)=="string" then
			s=assert(code.shaders[sname])
		else
			s=sname
			if not code.shaders[s] then
				local idx=#code.shaders+1
				code.shaders[s.name or idx]=s
				code.shaders[s]=s.name or idx
			end
			sname=s.name or ("__inline__shader__"..code.shaders[s])
		end
		
		if s[0] then return s[0] end

print("Compiling shader "..sname)
		s[0]=gl.CreateShader(stype)
		gl.ShaderSource(s[0],s.source)
		gl.CompileShader(s[0])
		
		if gl.GetShader(s[0], code.COMPILE_STATUS) == code.FALSE then -- error

			error( "failed to build shader " .. sname .. "\nSHADER COMPILER ERRORS\n\n" .. (gl.GetShaderInfoLog(s[0]) or "stoopid droid") .. "\n\n" )
		end
	
		return s[0]
	end
	
	local pbase={}
	local pmeta={__index=pbase}
	
	function code.program(pname)
		local p
		
		if type(sname)=="string" then
			p=assert(code.programs[pname])
		else
			p=pname
			if not code.programs[p] then
				local idx=#code.programs+1
				code.programs[p.name or idx]=p
				code.programs[p]=p.name or idx
			end
			pname=p.name or ("__inline__program__"..code.programs[p])
		end
		
		if not p[0] then
			setmetatable(p,pmeta)

			p.vars={}
			p[0]=gl.CreateProgram()
			
			for i,v in ipairs(p.vshaders) do
				gl.AttachShader( p[0] , code.shader(gl.VERTEX_SHADER,v) )
			end
			for i,v in ipairs(p.fshaders) do
				gl.AttachShader( p[0] , code.shader(gl.FRAGMENT_SHADER,v) )
			end
			
print("Linking program "..pname)
			gl.LinkProgram(p[0])
		
			if gl.GetProgram(p[0], gl.LINK_STATUS) == gl.FALSE then -- error

				print( gl.GetProgramInfoLog(p[0]) , "\n" )

				error( "failed to build program "..pname )
			end
			
		end
		return p
	end
	
	function pbase.attrib(p,vname)
		local r=p.vars[vname]
		if r then return r end
		r=gl.GetAttribLocation(p[0],vname)
		p.vars[vname]=r
		return r
	end

	function pbase.uniform(p,vname)
		local r=p.vars[vname]
		if r then return r end
		r=gl.GetUniformLocation(p[0],vname)
		p.vars[vname]=r
		return r
	end
	
	return code
end


return glescode

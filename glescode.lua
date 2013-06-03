-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generate a place to compile gl code into
-- this also contains matrix manipulation functions in the style of normal gl
-- and a simple Color replacement that just caches the color here for later use

local bit=require("bit")

local wstr=require("wetgenes.string")

local tardis=require("wetgenes.tardis")
local tcore=require("wetgenes.tardis.core") -- TODO: patch this into the base tardis core...

local core=require("gles.core")


local glescode={}


-- please pass in the gles base you wish to use, eg gles.gles2
-- returns a state table with compiler functions and places to
-- keep your source.
function glescode.create(gl)

	local code={}
	for n,v in pairs(gl) do code[n]=v end
	
	code.cache={}
	code.cache.color=tcore.new_v4()
	
	function code.Color(...)
		tcore.set(code.cache.color,...) -- may not set anything if no arguments are given
		return code.cache.color -- safe way of getting this value (a 4 float userdata)
	end

-- easy to type color helpers, returns 4 **PREMULTIPLIED** floats from color u16 or u32
	function code.C4(c) -- 0xffff -- ARGB
		local r,g,b,a	
		a=bit.band(bit.rshift(c,12),0xf)
		r=bit.band(bit.rshift(c, 8),0xf)
		g=bit.band(bit.rshift(c, 4),0xf)
		b=bit.band(c,0xf)
		a=a/0xf
		return a*r/0xf,a*g/0xf,a*b/0xf,a
	end

	function code.C8(c) -- 0xffffffff -- AARRGGBB
		local r,g,b,a	
		a=bit.band(bit.rshift(c,24),0xff)
		r=bit.band(bit.rshift(c,16),0xff)
		g=bit.band(bit.rshift(c, 8),0xff)
		b=bit.band(c,0xff)
		a=a/0xff
		return a*r/0xff,a*g/0xff,a*b/0xff,a
	end


-- matrix functions

	function code.reset_stacks()
		code.stacks={}		
		code.stack_mode=nil
		code.stack=nil
		code.stack_matrix=nil
	end
	code.reset_stacks() -- setup

	function code.matrix(mode)
		local v=code.stacks[mode]
		return v[#v]
	end
	
	function code.MatrixMode(mode)
		code.stack_mode=mode
		code.stack=code.stacks[code.stack_mode]
		if not code.stack then -- create on use
			local m4=tcore.new_m4() tcore.m4_identity(m4)
			code.stack={ m4 }
			code.stacks[code.stack_mode]=code.stack
		end
		code.stack_matrix=assert(code.stack[#code.stack])
	end

-- returns a matrix that can later be used in LoadMatrix
	function code.SaveMatrix(...)
		local m4=tcore.new_m4() tcore.set(m4,code.stack_matrix,16)
		return m4
	end

	function code.LoadMatrix(...)
		tcore.set(code.stack_matrix,...)
	end

	function code.MultMatrix(a)
		tcore.m4_product_m4(code.stack_matrix,a,code.stack_matrix)
	end

-- we have our own majick code for this sort of thing
	function code.Frustum(...)
		error("frustrum not suported")
	end

	function code.LoadIdentity()
		tcore.m4_identity(code.stack_matrix)
	end

	function code.Translate(vx,vy,vz)
		tcore.m4_translate(code.stack_matrix,vx,vy,vz)
	end

	function code.Rotate(d,vx,vy,vz)
		tcore.m4_rotate(code.stack_matrix,d,vx,vy,vz)
	end

	function code.Scale(vx,vy,vz)
		tcore.m4_scale_v3(code.stack_matrix,vx,vy,vz)
	end

	function code.PushMatrix()		
		local m4=tcore.new_m4() tcore.set(m4,code.stack_matrix,16)
		code.stack[#code.stack+1]=m4
		code.stack_matrix=assert(code.stack[#code.stack])
	end

	function code.PopMatrix()
		code.stack[#code.stack]=nil -- remove topmost
		code.stack_matrix=assert(code.stack[#code.stack]) -- this will assert on too many pops
	end

-- compiler functions

-- function to provide simple source for a shader program
	function code.progsrc(name,vsource,fsource)
		if not code.programs[name] then -- only do once
			code.shaders["v_"..name]={source=vsource}
			code.shaders["f_"..name]={source=fsource}
			code.programs[name]={
				vshaders={"v_"..name},
				fshaders={"f_"..name},
			}
		end
	end

	code.shaders={}
	code.programs={}
	code.defines={}

-- default shader prefix to use when building
	code.defines.shaderprefix="#version 100\nprecision mediump float;\n"

	if core.fixed_pipeline_available then -- probably desktop GL so needs haxtbh
	
		code.defines.shaderprefix="#version 120\n"

	end
	
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

-- Uhm, we could really use a shader lint or precompiler at this point, why does such a thing not exist?
-- I dont normaly care for lint but it would *really* make sense here...

--print("Compiling shader "..sname)
		s[0]=gl.CreateShader(stype)
		gl.ShaderSource(s[0],wstr.macro_replace(s.source,code.defines))
		gl.CompileShader(s[0])
		
		if gl.GetShader(s[0], gl.COMPILE_STATUS) == gl.FALSE then -- error

			error( "failed to build shader " .. sname .. "\nSHADER COMPILER ERRORS\n\n" .. (gl.GetShaderInfoLog(s[0]) or "stoopid droid") .. "\n\n" )
		end
	
		return s[0]
	end
	
	local pbase={}
	local pmeta={__index=pbase}
	
	function code.program(pname)
		local p
		
		if type(pname)=="string" then
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

			p.cache={}
			p.vars={}
			p[0]=gl.CreateProgram()
			
			for i,v in ipairs(p.vshaders) do
				gl.AttachShader( p[0] , code.shader(gl.VERTEX_SHADER,v) )
			end
			for i,v in ipairs(p.fshaders) do
				gl.AttachShader( p[0] , code.shader(gl.FRAGMENT_SHADER,v) )
			end
			
--print("Linking program "..pname)
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

-- internal cache helpers

	local function cache_set_v4(p,n,v)
		if not p.cache[n] then
			p.cache[n]=tcore.new_v4()
		end
		tcore.set(p.cache[n],v)
		return p.cache[n]
	end
	local function cache_set_m4(p,n,v)
		if not p.cache[n] then
			p.cache[n]=tcore.new_m4()
		end
		tcore.set(p.cache[n],v)
		return p.cache[n]
	end

	local function cache_check_v4(p,n,v)
		if not p.cache[n] then return false end
		return tcore.compare(p.cache[n],v)
	end
	local function cache_check_m4(p,n,v)
		if not p.cache[n] then return false end
		return tcore.compare(p.cache[n],v)
	end

-- set attribs
-- probably too hard to try and cache these here
-- due to the nature of vertexbuffers

 	function pbase.attrib_ptr(p,vname,size,gltype,normalize,stride,ptr)
		local n=p:attrib(vname)
		gl.VertexAttribPointer(n,vname,size,gltype,normalize,stride,ptr)
	end
	function pbase.attrib_stream(p,vname,onoff)
		local n=p:attrib(vname)
		if onoff then
			gl.EnableVertexAttribArray(n)
		else
			gl.DisableVertexAttribArray(n)
		end
	end
	function pbase.attrib_v4(p,vname,v4)
		local n=p:attrib(vname)
		gl.VertexAttrib4f(n,v4)
	end

-- set uniform values
-- try and cache these values to prevent updates which often seem rather expensive

	function pbase.uniform_v4(p,vname,v4)
		if cache_check_v4(p,vname,v4) then return end
		local n=p:uniform(vname)
		gl.Uniform4f(n, cache_set_v4(p,vname,v4) )
	end
	function pbase.uniform_m4(p,vname,m4)
		if cache_check_m4(p,vname,m4) then return end
		local n=p:uniform(vname)
		gl.UniformMatrix4f(n,cache_set_m4(p,vname,m4))
	end
	
	return code
end


return glescode

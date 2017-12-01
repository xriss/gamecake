-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generate a place to compile gl code into
-- this also contains matrix manipulation functions in the style of normal gl
-- and a simple Color replacement that just caches the color here for later use

local bit=require("bit")

local wstr=require("wetgenes.string")

local tardis=require("wetgenes.tardis")
--local tcore=require("wetgenes.tardis.core") -- TODO: patch this into the base tardis core...

local glslang=require("glslang")


local core=require("gles.core")


local glescode={}


-- please pass in the gles base you wish to use, eg gles.gles2
-- returns a state table with compiler functions and places to
-- keep your source.
function glescode.create(gl)

	local code={}
	for n,v in pairs(gl) do code[n]=v end
	
	code.cache={}
--	code.cache.color=tcore.new_v4()
	code.cache.color=tardis.v4.new()
	
	function code.Color(...)
--		tcore.set(code.cache.color,...) -- may not set anything if no arguments are given
		code.cache.color:set(...) -- may not set anything if no arguments are given
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
--			local m4=tcore.new_m4() tcore.m4_identity(m4)
			local m4=tardis.m4.new():identity()
			code.stack={ m4 }
			code.stacks[code.stack_mode]=code.stack
		end
		code.stack_matrix=assert(code.stack[#code.stack])
	end

-- returns a matrix that can later be used in LoadMatrix
	function code.SaveMatrix(...)
--		local m4=tcore.new_m4() tcore.set(m4,code.stack_matrix,16)
		local m4=tardis.m4.new(code.stack_matrix)
		return m4
	end

-- create a new matrix to mult by
	function code.CreateMatrix(a)
--		local m4=tcore.new_m4() tcore.set(m4,a,16)
		local m4=tardis.m4.new(a)
		return m4
	end

	function code.LoadMatrix(...)
--		tcore.set(code.stack_matrix,...)
		code.stack_matrix:set(...)
	end

	function code.MultMatrix(a)
--		tcore.m4_product_m4(code.stack_matrix,a,code.stack_matrix)
		code.stack_matrix:product(a,code.stack_matrix)
	end

	function code.PreMultMatrix(a)
--		tcore.m4_product_m4(a,code.stack_matrix,code.stack_matrix)
		a:product(code.stack_matrix,code.stack_matrix)
	end

-- we have our own majick code for this sort of thing
	function code.Frustum(...)
		error("frustrum not suported")
	end

	function code.LoadIdentity()
--		tcore.m4_identity(code.stack_matrix)
		code.stack_matrix:identity()
	end

	function code.Translate(vx,vy,vz)
--		tcore.m4_translate(code.stack_matrix,vx,vy,vz)
		code.stack_matrix:translate({vx,vy,vz})
	end

	function code.Rotate(d,vx,vy,vz)
--		tcore.m4_rotate(code.stack_matrix,d,vx,vy,vz)
		code.stack_matrix:rotate(d,{vx,vy,vz})
	end

	function code.Scale(vx,vy,vz)
--		tcore.m4_scale_v3(code.stack_matrix,vx,vy,vz)
		code.stack_matrix:scale_v3({vx,vy,vz})
	end

	function code.PushMatrix()		
--		local m4=tcore.new_m4() tcore.set(m4,code.stack_matrix,16)
		local m4=tardis.m4.new(code.stack_matrix)
		code.stack[#code.stack+1]=m4
		code.stack_matrix=assert(code.stack[#code.stack])
	end

	function code.PopMatrix()
		code.stack[#code.stack]=nil -- remove topmost
		code.stack_matrix=assert(code.stack[#code.stack]) -- this will assert on too many pops
	end


	function code.color_get_rgba()
--		return tcore.read(code.cache.color,1),tcore.read(code.cache.color,2),tcore.read(code.cache.color,3),tcore.read(code.cache.color,4)
		return code.cache.color[1],code.cache.color[2],code.cache.color[3],code.cache.color[4]
	end
	
-- apply MODELVIEW to a vertex, return the result
	function code.apply_modelview(va,vb,vc,vd)
		local t=type(va)
		if t=="number" then -- numbers
--			local v4=tcore.new_v4() tcore.set(v4,va,vb,vc,vd)
			local v4=tardis.v4.new(va,vb,vc,vd)
--			tcore.v4_product_m4(v4,code.matrix(gl.MODELVIEW))
			v4:product(code.matrix(gl.MODELVIEW))
--			return tcore.read(v4,1),tcore.read(v4,2),tcore.read(v4,3),tcore.read(v4,4)
			return v4[1],v4[2],v4[3],v4[4]
		elseif t=="table" then -- need to convert from a table
--			local v4=tcore.new_v4() tcore.set(v4,va,4)
			local v4=tardis.v4.new(va)
--			tcore.v4_product_m4(v4,code.matrix(gl.MODELVIEW))
			v4:product(code.matrix(gl.MODELVIEW))
--			va[1]=tcore.read(v4,1) va[2]=tcore.read(v4,2) va[3]=tcore.read(v4,3) va[4]=tcore.read(v4,4)
			va[1]=v4[1] va[2]=v4[2] va[3]=v4[3] va[4]=v4[4]
			return va
		else
--			tcore.v4_product_m4(va,code.matrix(gl.MODELVIEW))
			va:product(code.matrix(gl.MODELVIEW))
			return va
		end
	end


-- compiler functions
--
-- function to provide source for a named shader program
-- this adds any needed GLSL version header and defines
-- VERTEX_SHADER or FRAGMENT_SHADER appropriately
-- so only one source is required
--
	function code.program_source(name,vsource,fsource,filename)
--		if 	code.programs[name] then return code.programs[name] end -- only do once

		local aa=wstr.split(name,"?")
		local basename=aa[1]
		local params=aa[2]
		local paramdefs={"#define SHADER_NAME_"..basename:gsub("[^%w_]*","").." 1"}
		if params then -- query style
			for _,d in ipairs( wstr.split(params,"&") ) do
				local dd=wstr.split(d,"=")
				paramdefs[#paramdefs+1]="#define "..dd[1].." "..(dd[2] or "1")
			end
		end
		paramdefs=table.concat(paramdefs,"\n").."\n"
	
		local line=debug.getinfo(2).currentline+1 -- assume source is defined inline
		local vhead="#define VERTEX_SHADER 1\n"..paramdefs.."#line "..line.."\n"
		local fhead="#define FRAGMENT_SHADER 1\n"..paramdefs.."#line "..line.."\n"
		
		local copymerge=function(base,name,data)
			if not base[name] then base[name]={} end
			for n,v in pairs(data) do
				base[name][n]=v
			end
			return base[name]
		end

		local p=copymerge(code.programs,name,{
			vshaders={"v_"..name},
			fshaders={"f_"..name},
			name=name,
			vsource=vsource,
			fsource=fsource,
			filename=filename,
		})
		copymerge(code.shaders,"v_"..name,{ program=p, source=vhead..(vsource) })
		copymerge(code.shaders,"f_"..name,{ program=p, source=fhead..(fsource or vsource) }) -- single source trick

-- check that the code compiles OK right now?
		assert(code.shader(gl.VERTEX_SHADER,"v_"..name,filename))
		assert(code.shader(gl.FRAGMENT_SHADER,"f_"..name,filename))

		return p
	end

-- reset headers and load multiple shader sources from a single file
	code.headers={}
	function code.shader_sources(text,filename)
	
		local shaders=glslang.parse_chunks(text,filename,code.headers)

		for n,v in pairs(shaders) do
--print("PROGRAM",n,#v)
			code.program_source(n,table.concat(v,"\n"),nil,filename)
		end

	end

-- legacy version, obsolete, use the new program_source instead
-- this will be removed shortly
	function code.progsrc(name,vsource,fsource)
		if not code.programs[name] then -- only do once
			code.shaders["v_"..name]={ source=(vsource) }
			code.shaders["f_"..name]={ source=(fsource) }
			code.programs[name]={
				vshaders={"v_"..name},
				fshaders={"f_"..name},
			}
print("OBSOLETE","glescode.progsrc",name,#vsource,#fsource)
		end
	end

	code.shaders={}
	code.programs={}
	code.defines={}

-- default shader prefix to use when building

	if core.GLES2 then -- use GLES2 prefix
		code.defines.shaderprefix="#version 100\nprecision mediump float;\n"
	else
		code.defines_shaderprefix_tab={
			"#version 120\n", -- seems to work on osx?
			"#version 130\n", -- fails on osx?
--			"#version xxx\n", -- test fail case recovery
		}
		code.defines_shaderprefix_idx=#code.defines_shaderprefix_tab		
		code.defines.shaderprefix=code.defines_shaderprefix_tab[code.defines_shaderprefix_idx]

	end
	
-- forget cached info when we lose context, it is important to call this
	function code.forget()
--print("FORGETTING ALL SHADERS")
		for n,v in pairs(code.shaders) do
			if v[0] then
				if gl.IsShader(v[0]) then
					gl.DeleteShader(v[0])
				end
				v[0]=nil
			end
			if v.program and v.program.base then -- this can be regenerated
--print("DELETING SHADER "..n)
				code.shaders[n]=nil
			end
		end
--print("FORGETTING ALL PROGRAMS")
		for n,v in pairs(code.programs) do
			if v[0] then
				if gl.IsProgram(v[0]) then
					gl.DeleteProgram(v[0])
				end
				v[0]=nil
			end
			if v.base then -- this can be regenerated
--print("DELETING PROGRAM "..n)
				code.programs[n]=nil
			end
		end
	end
	
	function code.shader(stype,sname,filename)

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

--print("Compiling shader "..sname,wstr.dump(s))

		repeat local done=false

			s[0]=assert(gl.CreateShader(stype))

			local src=code.defines.shaderprefix..wstr.macro_replace(s.source,code.defines)
			
			gl.ShaderSource(s[0],src)
			gl.CompileShader(s[0])
			
			if gl.GetShader(s[0], gl.COMPILE_STATUS) == gl.FALSE then -- error
			
				if code.defines_shaderprefix_idx and code.defines_shaderprefix_idx>1 then -- try and brute force a working version number 

--print("Failed to build shader using prefix "..code.defines_shaderprefix_idx.." trying next prefix.")

					code.defines_shaderprefix_idx=code.defines_shaderprefix_idx-1
					code.defines.shaderprefix=code.defines_shaderprefix_tab[code.defines_shaderprefix_idx]

					gl.DeleteShader(s[0])
					
				else -- give up

					error( "failed to build shader " .. ( filename or "" ) .. " : " .. sname .. "\nSHADER COMPILER ERRORS\n\n" .. (gl.GetShaderInfoLog(s[0]) or "stoopid droid") .. "\n\n" )
					done=true
				
				end

			else
				done=true
			end
			
		until done
	
		return s[0]
	end
	
	local pbase={}
	local pmeta={__index=pbase}
	
	function code.program(pname)
		local p
		
		if type(pname)=="string" then
		
			p=code.programs[pname]
			if not p then -- try basename
				local basename=wstr.split(pname,"?")[1]
				local base=code.programs[basename]
				p=code.program_source(pname,base.vsource,base.fsource,base.filename)
				p.base=base
			end
			assert(p)
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
--			p.cache[n]=tcore.new_v4()
			p.cache[n]=tardis.v4.new()
		end
--		tcore.set(p.cache[n],v)
		p.cache[n]:set(v)
		return p.cache[n]
	end
	local function cache_set_m4(p,n,v)
		if not p.cache[n] then
--			p.cache[n]=tcore.new_m4()
			p.cache[n]=tardis.m4.new()
		end
--		tcore.set(p.cache[n],v)
		p.cache[n]:set(v)
		return p.cache[n]
	end

	local function cache_check_v4(p,n,v)
		if not p.cache[n] then return false end
--		return tcore.compare(p.cache[n],v)
		return p.cache[n]:compare(v)
	end
	local function cache_check_m4(p,n,v)
		if not p.cache[n] then return false end
--		return tcore.compare(p.cache[n],v)
		return p.cache[n]:compare(v)
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

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generate a place to compile gl code into
-- this also contains matrix manipulation functions in the style of normal gl
-- and a simple Color replacement that just caches the color here for later use

local bit=require("bit")

local wstr=require("wetgenes.string")

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local glslang=require("glslang")


local core=require("gles.core")


local glescode={}


-- please pass in the gles base you wish to use, eg gles.gles2
-- returns a state table with compiler functions and places to
-- keep your source.
function glescode.create(gl)

	local code={}
	for n,v in pairs(gl) do code[n]=v end
	
-- manage cached and stacked version of 			elseif name==gl.enable and related state
	code.state={}

	code.state.get=function(name)
		return code.state[code.state.index][name]
	end

-- mainly deal with the GL enable/disable state and associated values
-- possibly more should be added here?

	code.state.set=function(name,value)
	
		local top=code.state[code.state.index]

		if type(name)=="table" then -- multiple settings at once
			local it=name
			
			if it[gl.BLEND_SRC_RGB] and it[gl.BLEND_DST_RGB] and it[gl.BLEND_SRC_ALPHA] and it[gl.BLEND_DST_ALPHA] then
				if	top[gl.BLEND_SRC_RGB]~=it[gl.BLEND_SRC_RGB] or
					top[gl.BLEND_DST_RGB]~=it[gl.BLEND_DST_RGB] or
					top[gl.BLEND_SRC_ALPHA]~=it[gl.BLEND_SRC_ALPHA] or
					top[gl.BLEND_DST_ALPHA]~=it[gl.BLEND_DST_ALPHA] then
			
					gl.BlendFuncSeparate(
						it[gl.BLEND_SRC_RGB] ,
						it[gl.BLEND_DST_RGB] ,
						it[gl.BLEND_SRC_ALPHA] ,
						it[gl.BLEND_DST_ALPHA] )
						
					top[gl.BLEND_SRC_RGB]=it[gl.BLEND_SRC_RGB]
					top[gl.BLEND_DST_RGB]=it[gl.BLEND_DST_RGB]
					top[gl.BLEND_SRC_ALPHA]=it[gl.BLEND_SRC_ALPHA]
					top[gl.BLEND_DST_ALPHA]=it[gl.BLEND_DST_ALPHA]
				end
			end

			if it[gl.BLEND_EQUATION_RGB] and it[gl.BLEND_EQUATION_ALPHA] then
				if	top[gl.BLEND_EQUATION_RGB]~=it[gl.BLEND_EQUATION_RGB] or
					top[gl.BLEND_EQUATION_ALPHA]~=it[gl.BLEND_EQUATION_ALPHA] then

					gl.BlendEquationSeparate(
						it[gl.BLEND_EQUATION_RGB] ,
						it[gl.BLEND_EQUATION_ALPHA] )
				
					top[gl.BLEND_EQUATION_RGB]=it[gl.BLEND_EQUATION_RGB]
					top[gl.BLEND_EQUATION_ALPHA]=it[gl.BLEND_EQUATION_ALPHA]
				end
			end

			if it[gl.POLYGON_OFFSET_FACTOR] and it[gl.POLYGON_OFFSET_UNITS] then
				if	top[gl.POLYGON_OFFSET_FACTOR]~=it[gl.POLYGON_OFFSET_FACTOR] or
					top[gl.POLYGON_OFFSET_UNITS]~=it[gl.POLYGON_OFFSET_UNITS] then

					gl.PolygonOffset(
						it[gl.POLYGON_OFFSET_FACTOR] ,
						it[gl.POLYGON_OFFSET_UNITS] )

					top[gl.POLYGON_OFFSET_FACTOR]=it[gl.POLYGON_OFFSET_FACTOR]
					top[gl.POLYGON_OFFSET_UNITS]=it[gl.POLYGON_OFFSET_UNITS]
				end
			end

			if it[gl.SAMPLE_COVERAGE_VALUE] and it[gl.SAMPLE_COVERAGE_INVERT] then
				if	top[gl.SAMPLE_COVERAGE_VALUE]~=it[gl.SAMPLE_COVERAGE_VALUE] or
					top[gl.SAMPLE_COVERAGE_INVERT]~=it[gl.SAMPLE_COVERAGE_INVERT] then

					gl.SampleCoverage(
						it[gl.SAMPLE_COVERAGE_VALUE] ,
						it[gl.SAMPLE_COVERAGE_INVERT] )
						
					top[gl.SAMPLE_COVERAGE_VALUE]=it[gl.SAMPLE_COVERAGE_VALUE]
					top[gl.SAMPLE_COVERAGE_INVERT]=it[gl.SAMPLE_COVERAGE_INVERT]
				end
			end
					
			for n,v in pairs(it) do -- final settings recursive pass to pick up all other differences
				code.state.set(n,v)
			end

			return
		end
	
		if top[name]~=value then -- only hit the GL functions if a value has *actually* changed

			if name==gl.BLEND_COLOR then
				gl.BlendColor(value)

			elseif name==gl.BLEND_SRC_RGB then
				g.lBlendFuncSeparate(
					value,
					top[gl.BLEND_DST_RGB],
					top[gl.BLEND_SRC_ALPHA],
					top[gl.BLEND_DST_ALPHA])
					
			elseif name==gl.BLEND_SRC_ALPHA then
				g.lBlendFuncSeparate(
					top[gl.BLEND_SRC_RGB],
					value,
					top[gl.BLEND_SRC_ALPHA],
					top[gl.BLEND_DST_ALPHA])

			elseif name==gl.BLEND_DST_RGB then
				g.lBlendFuncSeparate(
					top[gl.BLEND_SRC_RGB],
					top[gl.BLEND_DST_RGB],
					value,
					top[gl.BLEND_DST_ALPHA])

			elseif name==gl.BLEND_DST_ALPHA then
				g.lBlendFuncSeparate(
					top[gl.BLEND_SRC_RGB],
					top[gl.BLEND_DST_RGB],
					top[gl.BLEND_SRC_ALPHA],
					value)

			elseif name==gl.BLEND_EQUATION_RGB then
				gl.BlendEquationSeparate(
					value,
					top[gl.BLEND_EQUATION_ALPHA])

			elseif name==gl.BLEND_EQUATION_ALPHA then
				gl.BlendEquationSeparate(
					top[gl.BLEND_EQUATION_RGB],
					value)
			
			elseif name==gl.CULL_FACE_MODE then
				gl.CullFace(value)

			elseif name==gl.COLOR_WRITEMASK then
				gl.ColorMask(value)

			elseif name==gl.DEPTH_WRITEMASK then
				gl.DepthMask(value)

			elseif name==gl.DEPTH_FUNC then
				gl.DepthFunc(value)
				
			elseif name==gl.DEPTH_RANGE then
				gl.DepthRange(value)

			elseif name==gl.POLYGON_OFFSET_FACTOR then
				gl.PolygonOffset(
					value,
					top[gl.POLYGON_OFFSET_UNITS])
					
			elseif name==gl.POLYGON_OFFSET_UNITS then
				gl.PolygonOffset(
					top[gl.POLYGON_OFFSET_FACTOR],
					value)

			elseif name==gl.SAMPLE_COVERAGE_VALUE then
				gl.SampleCoverage(
					value,
					top[gl.SAMPLE_COVERAGE_INVERT])

			elseif name==gl.SAMPLE_COVERAGE_INVERT then
				gl.SampleCoverage(
					top[gl.SAMPLE_COVERAGE_VALUE],
					value)

			elseif name==gl.SCISSOR_BOX then
				gl.Scissor(value)

			else -- default enable or disable
				if value and value~=0 then
					gl.Enable(name)
				else
					gl.Disable(name)
				end
			end

		end
		code.state[code.state.index][name]=value
	end

	code.state.push=function(it)
		local state={}
		for n,v in pairs( code.state[code.state.index] ) do
			state[n]=v
		end
		code.state.index=code.state.index+1
		code.state[code.state.index]=state -- new state is just a copy
		if it then code.state.set(it) end -- and make these state changes
	end
	
	code.state.pop=function()
		for n,v in pairs( code.state[code.state.index-1] ) do
			code.state.set(n,v) -- make any needed changes
		end
		code.state[code.state.index]=nil
		code.state.index=code.state.index-1
	end
	
	
	code.state_defaults=
	{
		[gl.CULL_FACE]					=	gl.FALSE,
		[gl.BLEND]						=	gl.TRUE,
		[gl.DITHER]						=	gl.TRUE,
		[gl.STENCIL_TEST]				=	gl.FALSE,
		[gl.DEPTH_TEST]					=	gl.FALSE,
		[gl.SCISSOR_TEST]				=	gl.FALSE,
		[gl.POLYGON_OFFSET_FILL]		=	gl.FALSE,
		[gl.SAMPLE_ALPHA_TO_COVERAGE]	=	gl.FALSE,
		[gl.SAMPLE_COVERAGE]			=	gl.FALSE,
		
		[gl.BLEND_COLOR]				=	V4(0,0,0,0),
		
		[gl.BLEND_SRC_RGB]				=	gl.ONE,
		[gl.BLEND_DST_RGB]				=	gl.ONE_MINUS_SRC_ALPHA,
		[gl.BLEND_SRC_ALPHA]			=	gl.ONE,
		[gl.BLEND_DST_ALPHA]			=	gl.ONE_MINUS_SRC_ALPHA,
		
		[gl.BLEND_EQUATION_RGB]			=	gl.FUNC_ADD,
		[gl.BLEND_EQUATION_ALPHA]		=	gl.FUNC_ADD,
		
		[gl.CULL_FACE_MODE]				=	gl.BACK,
		
		[gl.COLOR_WRITEMASK]			=	V4(gl.TRUE,gl.TRUE,gl.TRUE,gl.TRUE),

		[gl.DEPTH_WRITEMASK]			=	gl.TRUE,
		[gl.DEPTH_FUNC]					=	gl.LESS,
		[gl.DEPTH_RANGE]				=	V2(0,1),
		
		[gl.POLYGON_OFFSET_FACTOR]		=	0,
		[gl.POLYGON_OFFSET_UNITS]		=	0,
		
		[gl.SAMPLE_COVERAGE_VALUE]		=	1,
		[gl.SAMPLE_COVERAGE_INVERT]		=	gl.FALSE,
		
		[gl.SCISSOR_BOX]				=	V4(0,0,0,0),

-- ignore stencil for now as i cannot be bothered to type it all in, let alone test it...
--[[
		[gl.STENCIL_FUNC]				=	gl.ALWAYS,
		[gl.STENCIL_REF]				=	0,
		[gl.STENCIL_VALUE_MASK]			=	0xffffffff,
		[gl.STENCIL_BACK_FUNC]			=	gl.ALWAYS,
		[gl.STENCIL_BACK_REF]			=	0,
		[gl.STENCIL_BACK_VALUE_MASK]	=	0xffffffff,
]]

	}

	code.state.index=1
	code.state[1]={}
	for n,v in pairs(code.state_defaults) do
		code.state[code.state.index][n]=v
	end

	-- fix initial values that deviate from opengl defaults
	gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
	gl.Enable(gl.BLEND)
	-- from this point on you must only use code.state.set function not gl.Enable / etc
	-- or we will get out of sync

	code.cache={}
	code.cache.color=V4()
	
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

-- use generic tardis based stack code

-- reset all stacks
	function code.reset_stacks()
		code.stacks={}		
		code.stack_mode=nil
		code.stack=nil
	end
	code.reset_stacks() -- setup

-- get the current matrix for the given mode
	function code.matrix(mode)
		local v=code.stacks[mode]
		return v[#v]
	end

-- switch to the given mode
	function code.MatrixMode(mode)
		code.stack_mode=mode
		code.stack=code.stacks[code.stack_mode]
		if not code.stack then -- create on use
			code.stack=tardis.m4_stack()
			code.stacks[code.stack_mode]=code.stack
		end
	end


-- returns a matrix that can later be used in LoadMatrix
	function code.SaveMatrix()
		return code.stack.save()
	end

-- create a new matrix to be used in code.MultMatrix
	function code.CreateMatrix(a)
		return tardis.m4.new(a)
	end

	function code.LoadMatrix(...)
		code.stack.load(...)
	end

	function code.LoadIdentity()
		code.stack.identity()
	end

	function code.MultMatrix(a)
		code.stack.product(a)
	end

	function code.PreMultMatrix(a)
		code.stack.premult(a)
	end

	function code.Translate(...)
		code.stack.translate(...)
	end

	function code.PreTranslate(...)
		code.stack.pretranslate(...)
	end

	function code.Rotate(...)
		code.stack.rotate(...)
	end

	function code.Scale(...)
		code.stack.scale(...)
	end

	function code.PushMatrix()		
		code.stack.push()
	end

	function code.PopMatrix()
		code.stack.pop()
	end

-- we have our own majick code for this sort of thing
	function code.Frustum(...)
		error("frustrum not suported")
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
--		assert(code.shader(gl.VERTEX_SHADER,"v_"..name,filename))
--		assert(code.shader(gl.FRAGMENT_SHADER,"f_"..name,filename))

		return p
	end

-- reset headers at startup
	code.headers={}

-- load multiple shader sources from a single file
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
		
		local versions,src=glslang.yank_shader_versions( wstr.macro_replace(s.source,code.defines) )
		local errors={}
		local done=false
		for vi,version in ipairs(versions) do
			if not done then
				s[0]=assert(gl.CreateShader(stype))
				gl.ShaderSource(s[0],version..src)
				gl.CompileShader(s[0])
				if gl.GetShader(s[0], gl.COMPILE_STATUS) == gl.FALSE then -- error
					local err=gl.GetShaderInfoLog(s[0]) or "NIL"
					errors[#errors+1]="ERROR failed to build shader " .. ( filename or "" ) .. " : " .. sname .. "\nSHADER COMPILER ERRORS\n\n" .. err .. "\n\n"
--					..version..src.."\n\n"
					if vi<#versions then -- try again
						gl.DeleteShader(s[0])
						s[0]=nil
					end
				else
					done=true -- success
				end
			end
		end
		if not done then -- report all errors
			for i,e in ipairs(errors) do print(e) end
		end

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
				local base=assert(code.programs[basename],basename)
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

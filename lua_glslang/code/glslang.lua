-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.glslang

Manage string embedding and the glsl compiler and language options.

This is somewhat separate to the main GLES code and can be used without 
linking to any system dependent GL libs.

]]


local wstr=require("wetgenes.string")


-- the core is optional
local core ; pcall(function() core=require("glslang.core") end)


local glslang={}


--[[#lua.glslang.yank_shader_versions

parse a shader source and yank any #version out of the source and into a table
this allows multiple attempts at compiling the same source using different #versions

]]
function glslang.yank_shader_versions(src,default_version)
	local versions={}
	
	local lines=wstr.split_lines(src)
	
	for i,line in ipairs(lines) do
		if line:sub(1,8):lower()=="#version" then
			local duplicate=false
			for i,v in ipairs(versions) do if v==line then duplicate=true end end -- ignore duplicates
			if not duplicate then versions[#versions+1]=line end
			lines[i]="\n" -- remove
		end
	end
	if not versions[1] then -- force a default if no version is found in source
		versions[1]=default_version or "#version 100\n"
	end
	-- parse possible versions and add #defines so we can slightly tweak shader code
	for i,line in ipairs(versions) do
		local words=wstr.split_words(line)
		if words[3]=="es" or words[2]=="100" then
			line=line.."#define VERSION_ES 1\n"	-- es flag
		end
		line=line.."#define VERSION_"..words[2].." 1\n"	-- boolean number
		line=line.."#define VERSION "..words[2].."\n"	-- version number
		versions[i]=line -- and replace
	end

	-- return list of possible version lines , source with all #version lines removed
	return versions,table.concat(lines)
end


--[[#lua.glslang.pp

	code,err = glslang.pp(code,err)

Preprocess a vertex/fragment and return the result.

returns nil,error on error

]]

glslang.pp=function(code)
	if not core then return "NOCORE","NOCORE" end

	return core.pp(code)
end


--[[#lua.glslang.lint_gles2

	verr,ferr = glslang.lint_gles2(vcode,fcode)

Try and compile a vertex/fragment shader pair and return any errors.

returns nil,nil on success or an error string if either shader fails to 
compile.

Note that the glslang compile step seems rather slow, not sure what it 
gets up to but do not consider using this if speed is important.

]]

glslang.lint_gles2=function(vcode,fcode)
	if not core then return "NOCORE","NOCORE" end

	local verr,ferr,lerr=core.lint_gles2(vcode,fcode)
	
	if lerr=="" then lerr=nil end -- hack bad return value?

	return verr,ferr,lerr
end



--[[#lua.glslang.parse_chunks

	shaders=glslang.shader_chunks(text,filename,headers,flags)

		load multiple shader chunks from a single file and return a lookup
		table of name=code for each shader we just loaded. These can then be
		compiled or whatever.
		
		set flags.headers_only to true if you only care about parsing headers
		for later inclusion and do not want to parse shader chunks.
		

	#SHADER "nameofshader"

		is used at the start of a line to say which chunk the following text
		should go into. This is technically the same as #header but makes it
		explicit that this should be all the code for a shader rather than just
		a piece.

	#HEADER "nameofheader"

		can be used to define part of a shader, which can later be included in
		another shader by using

	#INCLUDE "nameofheader"
	
		Will insert the chunk of text previously defined as a #SHADER or #HEADER

	#SHADER or #HEADER

		without a name can be used to ignore the next part of a file, any text
		at the start of a file before the first #SHADER or #HEADER is also
		ignored. This enables these chunks to exist inside comments in a lua or
		whatever file.

]]
glslang.parse_chunks=function(text,filename,headers,flags)
	headers=headers or {}
	flags=flags or {}
	local ss=wstr.split(text,"\n")
	local chunk
	for i,l in ipairs(ss) do
		local flag=l:sub(1,7):lower()
		if flag=="#header" or flag=="#shader" then -- new chunk must be at start of line
			chunk=nil -- forget last chunk
			local name=l:match([["([^"]+)"]]) -- get name from inside quotes
			if name then -- from now on all lines go into this chunk
				chunk={}
				chunk[#chunk+1]="#line "..(i+1)	-- remember the line number from this file

-- shaders and headers chunks all live in the same name space
				if flag=="#header" then
					if flags.shaders_only then
						chunk=nil
					else
						headers[name]=chunk
					end
				elseif flag=="#shader" then
					if flags.headers_only then
						chunk=nil
					else
						headers[name]=chunk
					end
				end

			end
		else
			if chunk then -- only if we are in a chunk
				chunk[#chunk+1]=l
			end
		end
	end		

-- turn back into strings
	for n,v in pairs(headers) do
		if type(v) == "table" then
			headers[n]=table.concat(v,"\n")
		end
	end

	return headers
end

--[[#lua.glslang.replace_include

process #include statements in src with strings found in the headers table

]]
function glslang.replace_includes(src,headers)

	local f=function()
		local count=0
		local lines=wstr.split_lines(src)
		for i,line in ipairs(lines) do
			if line:sub(1,8):lower()=="#include" then -- include a previously declared chunk
				local name=line:match([["([^"]+)"]]) -- get name from inside quotes
				assert(headers[name],"glslang header "..name.." not found")
				lines[i]=headers[name].."#line "..(i+1).."\n"	-- reset the line number from this file
				count=count+1
			end
		end
		src=table.concat(lines)
		return count
	end

	for i=1,100 do -- maximum recursion
		if f()==0 then return src end -- until no change
	end

	return src
end
	
return glslang

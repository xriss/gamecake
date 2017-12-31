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

	shaders=glslang.shader_chunks(text,filename,headers)

		load multiple shader chunks from a single file and return a 
		lookup table of name=code for each shader we just loaded. These 
		can then be compiled or whatever.

	#SHADER "nameofshader"

		is used at the start of a line to say which chunk the following 
		text should go into.

	#HEADER "nameofheader"

		can be used to define part of a shader, which can later be 
		included in another shader by using

	#INCLUDE "nameofheader"
	
		Will insert the chunk of text previously defined as a #SHADER 
		or #HEADER

	#SHADER or #HEADER

		without a name can be used to ignore the next part of a file, 
		any text at the start of a file before the first #SHADER or 
		#HEADER is also ignored. This enables these chunks to exist 
		inside comments in a lua or whatever file.

]]
glslang.parse_chunks=function(text,filename,headers)
	headers=headers or {}
	local ss=wstr.split(text,"\n")
	local shaders={}
	local chunk
	for i,l in ipairs(ss) do
		local flag=l:sub(1,7):lower()
		if flag=="#header" or flag=="#shader" then -- new chunk must be at start of line
			chunk=nil -- forget last chunk
			local name=l:match([["([^"]+)"]]) -- get name from inside quotes
			if name then -- from now on all lines go into this chunk
				chunk={}
				chunk[#chunk+1]="#line "..(i+1)	-- remember the line number from this file
				if flag=="#header" then
					headers[name]=chunk
				elseif flag=="#shader" then
					shaders[name]=chunk
				end
			end
		else
			if chunk then -- only if we are in a chunk
				if l:sub(1,8):lower()=="#include" then -- include a previously declared chunk
					local name=l:match([["([^"]+)"]]) -- get name from inside quotes
					assert(headers[name],"glslang header "..name.." not found")
					for _,line in ipairs(headers[name]) do  -- include lines
						chunk[#chunk+1]=line
					end
					chunk[#chunk+1]="#line "..(i+1)	-- reset the line number from this file
				else
					chunk[#chunk+1]=l
				end
			end
		end
	end		
	
	return shaders
end
	
	
return glslang

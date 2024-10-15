

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## code.glslang.lua_glslang_lint_gles2


	lua_glslang_lint_gles2(lua)

	lua inputs
		vertex code string
		fragment code string
		linker flag strinf

	lua returns
		vertex error string
		fragment error string
		linker error string

Compile a vertex shader and a fragment shader for GLES2, return 
nil,nil,nil for no errors or an error string for either phase if 
something went wrong.



## code.glslang.lua_glslang_pp


	lua_glslang_cpp(lua)

	lua inputs
		code string

	lua returns
		preprocesed code string or nil
		error string if code string is nil

Run the preprocesor on the given code string.



## lua.glslang


Manage string embedding and the glsl compiler and language options.

This is somewhat separate to the main GLES code and can be used without 
linking to any system dependent GL libs.



## lua.glslang.lint_gles2


	verr,ferr = glslang.lint_gles2(vcode,fcode)

Try and compile a vertex/fragment shader pair and return any errors.

returns nil,nil on success or an error string if either shader fails to 
compile.

Note that the glslang compile step seems rather slow, not sure what it 
gets up to but do not consider using this if speed is important.



## lua.glslang.parse_chunks


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



## lua.glslang.pp


	code,err = glslang.pp(code,err)

Preprocess a vertex/fragment and return the result.

returns nil,error on error



## lua.glslang.replace_include


process #include statements in src with strings found in the headers table



## lua.glslang.yank_shader_versions


parse a shader source and yank any #version out of the source and into a table
this allows multiple attempts at compiling the same source using different #versions

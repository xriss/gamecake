


---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.glslang


Manage string embedding and the glsl compiler and language options.

This is somewhat separate to the main GLES code and can be used without 
linking to any system dependent GL libs.



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

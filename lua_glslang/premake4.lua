

project "lua_glslang"
language "C++"
files {
	"glslang/glslang/include/**" ,
	"glslang/glslang/GenericCodeGen/**" ,
	"glslang/glslang/MachineIndependent/**" ,
	"glslang/glslang/OSDependent/*" ,
	"glslang/glslang/Public/**" ,
	"glslang/OGLCompilersDLL/**",
}

files {
	"glslang/glslang/OSDependent/Unix/ossource.cpp" ,
}


files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }


includedirs { "." }


--[[

I'm not saying that anything after C89 is a newfangled abomination but 
we encounter problems with everything due to this flag, not least that 
clang/gcc barfs on .c files if this flag is used and we lack enough 
flag control in premake4 to only use it on C++ files.

Currently "fixing" it by renaming our .c code to .cpp as the least 
worst option.

Ideally we should bump up to premake5 but I need to finish hacking 
together the lua deport of premake which would make that upgrade less 
of an issue and cleanup all the build hacks I currently have.

]]
buildoptions{ "-std=c++11" }



links { "lib_lua" }

KIND{kind="lua",name="glslang_core"}

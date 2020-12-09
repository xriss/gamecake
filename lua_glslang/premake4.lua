

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



links { "lib_lua" }

KIND{kind="lua",name="glslang_core",lua="glslang.core"}

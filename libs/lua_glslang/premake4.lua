

project "lua_glslang"
language "C++"
files {
	"glslang/glslang/Include/**" ,
	"glslang/glslang/CInterface/**" ,
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


includedirs { "." , "./glslang" , "./glslang/glslang/Include" }


buildoptions{"-std=c++11"}


links { "lib_lua" }

KIND{kind="lua",name="glslang_core",lua="glslang.core"}

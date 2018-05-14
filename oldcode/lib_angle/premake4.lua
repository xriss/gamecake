
project "lib_angle"
kind "StaticLib"
language "C++"

includedirs { ".","./include","./src"}

files { 
		"./src/common/**.cpp",
		"./src/compiler/**.cpp",
		"./src/libEGL/**.cpp",
		"./src/libGLESv2/**.cpp",
		"./src/third_party/**.cpp",
		"./src/libANGLE/*.cpp",
		"./src/libANGLE/renderer/*.cpp",
		"./src/libANGLE/renderer/d3d/*.cpp",
		"./src/libANGLE/renderer/d3d/d3d9/**.cpp",
}

buildoptions { " -std=c++11" }


KIND{}

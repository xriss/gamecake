
project "lua_speak"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }


files { "flite/**.cpp" , "flite/**.c" , "flite/**.h" }



includedirs { "." , "flite/include" , "flite/voices/usenglish" , "flite/voices/cmulex" }


KIND{lua="wetgenes.speak.core"}


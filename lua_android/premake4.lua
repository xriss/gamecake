

if false and ANDROID then


project "lua_android"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.cc" , "code/**.h" , "all.h" }


includedirs { "." }

--links { "lua_main" , "lua" }

defines { "LUA_LIB" }


SET_KIND("ConsoleApp","nacl","nacl")
SET_TARGET("","lua.nexe",true)


end




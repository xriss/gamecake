

if NACL then


project "lua_nacl"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.cc" , "code/**.h" , "all.h" }

links { "ppapi" ,  "ppapi_cpp" , "pthread" , "srpc" }

includedirs { "." }

--links { "lib_lua" , "lua" }

defines { "LUA_LIB" }


SET_KIND("ConsoleApp","nacl","nacl")
SET_TARGET("","lua.nexe",true)


end





project "lua_gamecake"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." }
includedirs { "../lib_hacks/code" }

if RASPI or GAMECAKE_WIN_TYPE=="raspi" then

	includedirs { "/opt/vc/include" }

end

KIND{lua="wetgenes.gamecake.core"}


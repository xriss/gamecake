
project "lua_gamecake"
language "C"

files { "code/**.cpp" , "code/**.c" , "code/**.h" , "cache/**.c"  }

links { "lib_lua" }


includedirs { "." , "./code" }
includedirs { "../lib_hacks/code" }

defines { "WETGENES_CACHE" }

if RASPI or GAMECAKE_WIN_TYPE=="raspi" then

	includedirs { "/opt/vc/include" }

end

links(static_lib_names)


KIND{lua="wetgenes.gamecake.core"}

-- must refresh generated C here so it contains ourselves
dofile("cache/cache.lua")
dofile("cache/preloadlibs.lua")


include("zlib")


project "lua_zlib"
language "C"
files { "src/lua_zlib.c" }

links { "lua51" , "lua_zlib_zlib" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." , "zlib" }


SET_KIND("lua","zlib","zlib")
SET_TARGET("","zlib")


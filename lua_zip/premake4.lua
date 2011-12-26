
include("zziplib")


project "lua_zip"
language "C"
files { "src/**.c" , "src/**.h" }

links { "lib_lua" , "lua_zip_zziplib" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." , "zziplib" }


SET_KIND("lua","zip","zip")
SET_TARGET("","zip")


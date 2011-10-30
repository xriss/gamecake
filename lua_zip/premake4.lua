
include("zziplib")


project "lua_zip"
language "C++"
files { "src/**.cpp" , "src/**.c" , "src/**.h" }

links { "lua51" , "lua_zip_zziplib" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." , "zziplib" }


SET_KIND("lua","zip","zip")
SET_TARGET("","zip")


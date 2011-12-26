
project "lua_zip"
language "C"
files { "src/**.c" , "src/**.h" }

links { "lib_lua" , "lib_zzip" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." , "../lib_zzip" }


SET_KIND("lua","zip","zip")
SET_TARGET("","zip")


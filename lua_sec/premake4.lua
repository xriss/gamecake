

project "lua_sec"
language "C++"
files { "src/**.c" , "src/**.cpp" , "src/**.h" }

links { "lib_lua" }

if os.get() == "windows" then

	excludes("src/usocket.*")
	
	links { "WS2_32" , "libcrypto" , "libssl" }
	
	includedirs "openssl/include"
	libdirs "openssl/lib"

else -- nix

	excludes("src/wsocket.*")

	links { "crypt" , "ssl" }

end


SET_KIND("lua","ssl.core","ssl_core")
SET_TARGET("/ssl","core")




project "lua_sec"
language "C++"
files { "src/**.c" , "src/**.cpp" , "src/**.h" }

links { "lib_lua" }

	includedirs "../lib_openssl/include"


-- this is all just copies of lua socket, so we do not really need?

	excludes("src/usocket.*")
	excludes("src/wsocket.*")
	excludes("src/timeout.*")
	excludes("src/buffer.*")


if WINDOWS then

	excludes("src/usocket.*")
	
--	links { "WS2_32" , "libcrypto" , "libssl" }
--	libdirs "openssl/lib"

else -- nix

	excludes("src/wsocket.*")

--	links { "crypt" , "ssl" }

end


KIND{kind="lua",name="ssl.core",luaname="ssl.core",luaopen="ssl_core"}



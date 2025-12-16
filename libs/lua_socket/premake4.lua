
project "lua_socket"
language "C"
files { "git/src/*.c" , "git/src/*.cpp" , "git/src/*.h" }
excludes("git/src/mime.*")
excludes("git/src/unix.*")

links { "lib_lua" }

--defines "LUA_VERSION_NUM=501"
defines "LUASOCKET_API=extern"
defines "LUASOCKET_EXPORTS"

if WINDOWS then

	defines "LUASOCKET_INET_PTON"

	excludes("git/src/usocket.*")
	excludes("git/src/serial.*")
	excludes("git/src/unix*")

	links { "ws2_32" }
	
else -- nix

	excludes("git/src/wsocket.*")

	links { "crypt" , "ssl" }

end


KIND{kind="lua",dir="socket",name="core",luaname="socket.core",luaopen="socket_core"}




project "lua_mime"
language "C"
files { "git/src/mime.c" ,"git/src/mime.h" , "git/src/compat.c" , "git/src/compat.h" }

links { "lib_lua" }

--defines "LUA_VERSION_NUM=501"
defines "MIME_API=extern"
defines "MIME_EXPORTS"

KIND{kind="lua",dir="mime",name="core",luaname="mime.core",luaopen="mime_core"}



if not NACL then -- just grab mime under nacl

project "lua_socket"
language "C"
files { "git/src/**.c" , "git/src/**.cpp" , "git/src/**.h" }
excludes("git/src/mime.*")
excludes("git/src/unix.*")

links { "lib_lua" }

if WINDOWS then

	excludes("git/src/usocket.*")

	links { "ws2_32" }

	defines "LUASOCKET_EXPORTS"
	
	if MINGW then -- commandline escape problems?
		defines "LUASOCKET_API=__declspec\\(dllexport\\)"
	else
		defines "LUASOCKET_API=__declspec(dllexport)"
	end
	
else -- nix

	excludes("git/src/wsocket.*")

	links { "crypt" , "ssl" }

end


KIND{kind="lua",dir="socket",name="core",luaname="socket.core",luaopen="socket_core"}

end


project "lua_mime"
language "C++"
files { "git/src/mime.c" ,"git/src/mime.h" }

links { "lib_lua" }

if WINDOWS then

	defines "MIME_EXPORTS"
	if MINGW then -- commandline escape problems?
		defines "MIME_API=__declspec\\(dllexport\\)"
	else
		defines "MIME_API=__declspec(dllexport)"
	end

else -- nix

end

KIND{kind="lua",dir="mime",name="core",luaname="mime.core",luaopen="mime_core"}


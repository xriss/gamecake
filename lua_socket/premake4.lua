
if not NACL then -- just grab mime under nacl

project "lua_socket"
language "C++"
files { "src/**.c" , "src/**.cpp" , "src/**.h" }
excludes("src/mime.*")
excludes("src/unix.*")

links { "lib_lua" }

if WINDOWS then

	excludes("src/usocket.*")

	links { "ws2_32" }

	defines "LUASOCKET_EXPORTS"
	
	if MINGW then -- commandline escape problems?
		defines "LUASOCKET_API=__declspec\\(dllexport\\)"
	else
		defines "LUASOCKET_API=__declspec(dllexport)"
	end
	
else -- nix

	excludes("src/wsocket.*")

	links { "crypt" , "ssl" }

end


KIND{kind="lua",dir="socket",name="core",luaname="socket.core",luaopen="socket_core"}

end


project "lua_mime"
language "C++"
files { "src/mime.c" ,"src/mime.h" }

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


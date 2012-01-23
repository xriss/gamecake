
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


SET_KIND("lua","socket.core","socket_core")
SET_TARGET("/socket","core")



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

SET_KIND("lua","mime.core","mime_core")
SET_TARGET("/mime","core")


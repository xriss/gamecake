project "lua_sys"
language "C"
files { "./src/luasys.c" }
includedirs { "./src" }
KIND{lua="sys"}

if not NACL then
project "lua_sys_sock"
language "C"
files { "./src/sock/sys_sock.c" }
includedirs { "./src" }
KIND{lua="sys.sock"}
end

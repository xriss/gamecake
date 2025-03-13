project "lua_sys"
language "C"
files { "luasys/src/luasys.c" }
includedirs { "luasys/src" }
KIND{lua="sys"}

project "lua_sys_sock"
language "C"
files { "luasys/src/sock/sys_sock.c" }
includedirs { "luasys/src" }
KIND{lua="sys.sock"}

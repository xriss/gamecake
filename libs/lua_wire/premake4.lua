
project "lua_wire"
language "C"

includedirs { "." , "./code" }
includedirs { "c11threads/git" }

files { "code/lua_wire.c" }


-- need windows thread hax
if WINDOWS then
files { "c11threads/git/c11threads_win32.c" }
end

links { "lib_lua" }

KIND{lua="wetgenes.wire.core"}


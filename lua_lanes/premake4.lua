
project "lua_lanes"
language "C"
files { "src/**.cpp" , "src/**.c" , "src/**.h" }

if NIX then
--defines { "USE_PTHREAD_TIMEDJOIN" }
end

links { "lib_lua" }

KIND{lua="lanes.core"}

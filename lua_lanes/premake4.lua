
project "lua_lanes"
language "C"
files { "lanes/src/**.c" , "lanes/src/**.h" , "lanes/src/**.cpp" }

--if NIX then
--defines { "USE_PTHREAD_TIMEDJOIN" }
--end

--buildlinkoptions { "-std=c++17" }


links { "lib_lua" }

KIND{lua="lanes.core"}


project "lua_lanes"
language "C"
files { "lanes/src/**.c" , "lanes/src/**.h" }

--if NIX then
--defines { "USE_PTHREAD_TIMEDJOIN" }
--end

buildlinkoptions { "-Wno-implicit-function-declaration" }


links { "lib_lua" }

KIND{lua="lanes.core"}

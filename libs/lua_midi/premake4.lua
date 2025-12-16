

project "lua_midialsa"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" }

-- this lib is not thread safe so will need to be tweaked...

links { "lib_lua" }

KIND{kind="lua",name="midi_alsa_core"}


project "lua_box2d"
language "C++"
files { "code/**.cpp" , "code/**.h" , "Box2D/Source/**.cpp" , "Box2D/Source/**.h" }

includedirs { "Box2D/Source" , "Box2D/Include" }

links { "lib_lua" }

KIND{kind="lua",dir="box2d",name="core",luaname="box2d.core",luaopen="box2d_core"}


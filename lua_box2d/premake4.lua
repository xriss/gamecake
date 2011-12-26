
project "lua_box2d"
language "C++"
files { "code/**.cpp" , "code/**.h" , "Box2D/Source/**.cpp" , "Box2D/Source/**.h" }

includedirs { "Box2D/Source" , "Box2D/Include" }

links { "lib_lua" }

SET_KIND("lua","box2d.core","box2d_core")
SET_TARGET("/box2d","core")


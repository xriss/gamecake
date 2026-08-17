

project "lua_bullet"
language "C++"
files {
	"src/Bullet3Common/**" ,
	"src/Bullet3Geometry/**" ,
	"src/Bullet3Serialize/**" ,
	"src/BulletDynamics/**" ,
	"src/BulletSoftBody/**" ,
	"src/Bullet3Collision/**" ,
	"src/Bullet3Dynamics/**" ,
	"src/BulletCollision/**" ,
	"src/BulletInverseDynamics/**" ,
	"src/LinearMath/**" ,
}

files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

includedirs { "." , "./src" }



defines {"BT_USE_DOUBLE_PRECISION"}



links { "lib_lua" }

KIND{lua="wetgenes.bullet.core"}

--[[

We currently need a small hack to better support of all things bullet 
like objects...

I want to switch to jolt that seems more active and game focused, 
technically we have no actually released/finished code running on this 
lib so should be fine...

Also we need to hack water in manually... Jolt has some builtin so that 
would be nice.

https://github.com/jrouwe/joltphysics


]]

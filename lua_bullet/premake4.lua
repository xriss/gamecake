

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

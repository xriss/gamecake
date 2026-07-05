
project "lib_box2d"
language "C"
includedirs { "git/include" }
files { "git/src/*.c" , "git/src/*.h" }

if EMCC then

defines { "BOX2D_DISABLE_SIMD" }

end

KIND{}

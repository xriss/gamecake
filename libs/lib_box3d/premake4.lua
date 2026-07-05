
project "lib_box3d"
language "C"
includedirs { "git/include" }
files { "git/src/*.c" , "git/src/*.h" }


if EMCC then

defines { "BOX3D_DISABLE_SIMD" }

end

KIND{}

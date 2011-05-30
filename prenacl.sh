
#simple forced nacl build, just to see if we can get somewhere with this

export PATH=../nacl/toolchain/linux_x86/nacl/bin:$PATH

#so premake knows that it is a nacl build
build/premake4 gmake nacl

#perform a build
cd build-gmake-nacl
make $*
cd ..


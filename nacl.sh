cd `dirname $0`

#simple forced nacl build, just to see if we can get somewhere with this

naclsdk=../naclsdk
export PATH=$naclsdk/toolchain/linux_x86/nacl/bin:$PATH

#so premake knows that it is a nacl build
build/premake4 gmake nacl

#perform a build
cd build-gmake-nacl

if [ "$1" == "release" ] ; then
	make config=release
else
	make $*
fi

cd ..


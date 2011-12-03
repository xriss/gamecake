cd `dirname $0`

abi=../../sdks/android-8-arm/arm-linux-androideabi
export PATH=$abi/bin:$PATH
export GCCLIB=$abi/lib/libstdc++.a

#perform a build
cd build-gmake-android

if [ "$1" == "release" ] ; then
	make -n config=release
else
	make -n $*
fi

which cc
which as
cc -DANDROID -I../lua/src -I../lua/src   -o "obj/Debug/lua51/lzio.o" -c "../lua/src/lzio.c"

cd ..

cd android

#ant debug install

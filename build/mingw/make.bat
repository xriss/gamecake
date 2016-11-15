cd `dirname $0`
cd ..

if [ "$1" = "clean" ] ; then
	rm -rf build-gmake-windows-native
	exit 0
fi

build/premake4.exe gmake mingwin

cd build-gmake-windows-native

if [ "$1" = "debug" ] ; then
	make config=debug_mingwin $2 $3 $4 || exit 1
else
	make config=release_mingwin $2 $3 $4 || exit 1
fi

cd ..


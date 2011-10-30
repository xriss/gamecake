cd `dirname $0`
build/premake4 gmake
cd build-gmake

if [ "$1" == "release" ] ; then
	make config=release
else
	make $*
fi

cd ..


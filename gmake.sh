cd `dirname $0`
build/premake4 gmake
cd build-gmake
make $*
cd ..


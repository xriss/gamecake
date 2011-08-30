cd `dirname $0`
cd ../../bin/exe
./lua -e'dofile("../lua/apps.lua");apps.start("wettest");' $*

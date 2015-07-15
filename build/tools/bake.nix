cd `dirname $0`
cd premake4/build/gmake.unix
make
cd ../../..
cp premake4/bin/release/premake4 ../premake4.nix
rm -rf premake4
hg revert premake4



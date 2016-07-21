cd `dirname $0`

#prepare
../../bin/revert_exe

#build windows

../../lua/mingw/make clean
../../lua/mingw/make release gamecake

#build raspi

../../lua/vbox_raspi/make

#build nacl

../../lua/nacl/make clean
../../lua/nacl/make release gamecake

#build emscipten

../../lua/emcc/make clean
../../lua/emcc/make release gamecake

# build 64bit ubuntu 12.04 version

../../lua/vbox_1204_64/make

#build 32bit ubuntu 12.04 version 

../../lua/vbox_1204_32/make

#build osx version 64 (dont think anything uses 32 anymore)

../../lua/vbox_osx/make

#compress files

upx -9 ../../bin/exe/*


#show status, this should list new versions of everything we just built

hg status -R ../../bin


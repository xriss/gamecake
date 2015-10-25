cd `dirname $0`

#prepare
../../bin/revert_exe

#build windows

../../lua/mingw/make clean
../../lua/mingw/make release gamecake

#build raspi

../../lua/raspi/make clean
../../lua/raspi/make release gamecake

#build nacl

../../lua/nacl/make clean
../../lua/nacl/make release gamecake

# build 64bit ubuntu 12.04 version

../../lua/vbox_1204_64/make

#build 32bit ubuntu 12.04 version using schroot

../../lua/vbox_1204_32/make

#build on remote mac machine and grab the binary back to this system

../../lua/vbox_osx/make

#compress files

upx -9 ../../bin/exe/*


#show status, this should list new versions of everything we just built

hg status -R ../../bin


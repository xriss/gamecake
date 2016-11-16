cd `dirname $0`

#prepare
#../exe/revert_exe

#build windows

./mingw/make clean
./mingw/make release gamecake

#build nacl

./nacl/make clean
./nacl/make release gamecake

#build emscipten

./emcc/make clean
./emcc/make release gamecake

# build 64bit ubuntu 12.04 version

../vbox_1204_64/make

#build 32bit ubuntu 12.04 version 

../vbox_1204_32/make

#build osx version 64 (dont think anything uses 32 anymore)

#../vbox_osx/make

#build raspi

#../vbox_raspi/make

#compress files

upx -9 ../exe/*


#show status, this should list new versions of everything we just built

cd ../exe

git status

#add and commit changed files
./update_files


cd sdl2_osx

./configure --enable-static

make

cd ..

#prevent dynamic linking
#rm sdl2_osx/build/.libs/*.dylib

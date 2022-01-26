
cd sdl2_x64

./configure --enable-static --disable-esd --disable-arts --disable-video-directfb --disable-rpath --enable-alsa --enable-alsa-shared --enable-pulseaudio --enable-pulseaudio-shared --enable-x11-shared --enable-sdl-dlopen --disable-input-tslib

make

cd ..

#prevent dynamic linking
rm sdl2_x64/build/.libs/*.so

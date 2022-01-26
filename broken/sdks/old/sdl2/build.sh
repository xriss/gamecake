schroot -c precise_x32 -u kriss -- bash -c "cd sdl2_x32;./configure --enable-static --disable-esd --disable-arts --disable-video-directfb --disable-rpath --enable-alsa --enable-alsa-shared --enable-pulseaudio --enable-pulseaudio-shared --enable-x11-shared --enable-sdl-dlopen --disable-input-tslib"
schroot -c precise_x32 -u kriss -- bash -c "cd sdl2_x32;make"

schroot -c precise_x64 -u kriss -- bash -c "cd sdl2_x64;./configure --enable-static --disable-esd --disable-arts --disable-video-directfb --disable-rpath --enable-alsa --enable-alsa-shared --enable-pulseaudio --enable-pulseaudio-shared --enable-x11-shared --enable-sdl-dlopen --disable-input-tslib"
schroot -c precise_x64 -u kriss -- bash -c "cd sdl2_x64;make"

#prevent dynamic linking
#rm sdl2_x32/build/.libs/*.so
#rm sdl2_x64/build/.libs/*.so

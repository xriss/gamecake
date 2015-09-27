
# update and install some basic dependencies

sudo apt-get update

sudo apt-get install -y unzip clang build-essential libreadline-dev libgl1-mesa-dev libx11-dev libasound2-dev libudev-dev


# make a version of premake4 and install it

unzip /hg/lua/vbox_bins/premake-4.4-beta5-src.zip
cd premake-4.4-beta5/build/gmake.unix
make
cd ../..
sudo cp bin/Release/premake4 /usr/local/bin/


# make a version of SDL2 and install it

sudo apt-get install -y libfreeimage-dev libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libasound2-dev libjpeg8-dev libwebp-dev automake
tar zxvf /hg/lua/vbox_bins/SDL2-2.0.3.tar.gz
cd SDL2-2.0.3
./configure --enable-static --disable-esd --disable-arts --disable-video-directfb --disable-rpath --enable-alsa --enable-alsa-shared --enable-pulseaudio --enable-pulseaudio-shared --enable-x11-shared --enable-sdl-dlopen --disable-input-tslib
make && make install
cd ..



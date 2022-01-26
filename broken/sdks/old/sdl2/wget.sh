rm SDL2-2.0.3.zip
rm -rf sdl2_x32
rm -rf sdl2_x64

wget https://www.libsdl.org/release/SDL2-2.0.3.zip

7z x SDL2-2.0.3.zip
mv SDL2-2.0.3 sdl2_x32

7z x SDL2-2.0.3.zip
mv SDL2-2.0.3 sdl2_x64

7z x SDL2-2.0.3.zip
mv SDL2-2.0.3 sdl2_osx

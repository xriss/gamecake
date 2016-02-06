# sdl2 cmake project-config input for ./configure scripts

set(prefix "/buildbot/slave/SDL/sdl-raspberrypi/src/raspberrypi-buildbot/rpi-sdl2-installed") 
set(exec_prefix "${prefix}")
set(libdir "@libdir")
set(SDL2_PREFIX "/buildbot/slave/SDL/sdl-raspberrypi/src/raspberrypi-buildbot/rpi-sdl2-installed")
set(SDL2_EXEC_PREFIX "/buildbot/slave/SDL/sdl-raspberrypi/src/raspberrypi-buildbot/rpi-sdl2-installed")
set(SDL2_LIBDIR "${exec_prefix}/lib")
set(SDL2_INCLUDE_DIRS "${prefix}/include/SDL2")
set(SDL2_LIBRARIES "-L${SDL2_LIBDIR} -Wl,-rpath,${libdir} -lSDL2 ")

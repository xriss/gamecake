if [ "`uname`" = "Darwin" ] ; then
        echo osx unsuported, please build gamecake from source

elif [ "`uname -o`" = "Msys" ] ; then
        echo exe
	exe/gamecake.exe poopeepanda.fun.lua

elif [ "`uname -m`" = "aarch64" ] ; then
	exe/gamecake.a64 poopeepanda.fun.lua

elif [ "`uname -m`" = "armv7l" ] ; then
        echo rpi unsuported, please build gamecake from source

elif [ "`uname -m`" = "x86_64" ] ; then
        echo x64
	exe/gamecake.x64 poopeepanda.fun.lua

else
        echo error unknown arch, please build gamecake from source

fi

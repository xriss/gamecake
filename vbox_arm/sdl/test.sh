cd `dirname $0`

./ssh " gamecake/vbox_arm/sdl/make.sh $* "

scp -P 5522 pi@localhost:gamecake/vbox_arm/sdl/test test


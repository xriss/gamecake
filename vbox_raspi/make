cd `dirname $0`


./box-up-wait

./ssh " cd gamecake && ./git-pull && cd .. || git clone --recursive -v --progress https://github.com/xriss/gamecake.git "

./ssh " gamecake/build/make $* "

scp -P 5522 pi@localhost:gamecake/exe/gamecake.nix ../exe/gamecake.arm

./box-down


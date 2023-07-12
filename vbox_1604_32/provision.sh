
# update and install some basic dependencies

sudo apt-get update
sudo apt-get upgrade

#/gamecake/build/install
# use system versions of SDL2 and LuaJIT
sudo apt-get install -y libsdl2-dev
sudo apt-get install -y libluajit-5.1-dev 

sudo apt-get install -y lua5.1
sudo apt-get install -y lua-filesystem

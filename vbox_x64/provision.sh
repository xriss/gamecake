
# update and install some basic dependencies

sudo apt-get update

sudo /gamecake/install-build-dependencies
sudo /gamecake/build/install

sudo apt-get -y install clang-19
sudo apt-get -y remove clang
sudo ln -s /usr/bin/clang-19 /usr/bin/clang
sudo ln -s /usr/bin/clang++-19 /usr/bin/clang++
sudo ln -s /usr/bin/clang-cpp-19 /usr/bin/clang-cpp

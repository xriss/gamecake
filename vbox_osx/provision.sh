
# update and install some basic dependencies

#xcode-select --install


export PATH=/opt/local/bin:/usr/local/bin:$PATH

#must not be root, fix broken osx getopt

su vagrant -c " brew update-reset "

su vagrant -c " brew install gnu-getopt "
su vagrant -c " brew link --force gnu-getopt "
su vagrant -c " echo 'export PATH=\"/usr/local/opt/gnu-getopt/bin:$PATH\"' >> ~/.bash_profile "

su vagrant -c " brew install wget "

su vagrant -c " brew install luarocks "
su vagrant -c " luarocks install luafilesystem "


su vagrant -c " cd gamecake && ./git-pull && cd .. || git clone --recursive -v --progress https://github.com/xriss/gamecake.git "

su vagrant -c " . .bash_profile ; gamecake/build/install "

 


# update and install some basic dependencies

#xcode-select --install


export PATH=/opt/local/bin:/usr/local/bin:$PATH


/hg/lua/build/depends/install.osx


exit 0


brew update


curl -O https://distfiles.macports.org/MacPorts/MacPorts-2.3.3.tar.bz2
tar xjvf MacPorts-2.3.3.tar.bz2
cd MacPorts-2.3.3
./configure && make && sudo make install
cd ..
rm -rf MacPorts-2.3.3

export PATH=/opt/local/bin:$PATH
sudo port -v selfupdate

sudo port install bindfs

sudo mkdir /gamecake_fix
sudo bindfs -m vagrant /gamecake /gamecake_fix
cd /gamecake_fix

# OK we now have fixed permission and can hopefully build and install

/gamecake_fix/build/depends/install



# update and install some basic dependencies

#xcode-select --install


export PATH=/opt/local/bin:/usr/local/bin:$PATH

#must not be root, fix broken osx getopt
su vagrant -c " brew install gnu-getopt "
su vagrant -c " echo 'export PATH=\"/usr/local/opt/gettext/bin:$PATH\"' >> ~/.bash_profile "

su vagrant -c " /gamecake/build/install "

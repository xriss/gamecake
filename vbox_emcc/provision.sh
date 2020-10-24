
# update and install some basic dependencies

sudo apt -y update
sudo apt -y upgrade

#must not be root?
su vagrant -c /gamecake/build/install-emsdk /home/vagrant/


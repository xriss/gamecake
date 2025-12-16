
# update and install some basic dependencies

sudo apt -y update
sudo apt -y upgrade

#must not be root?
su vagrant -c "/gamecake/build/install-andsdk /home/vagrant/"

su vagrant -c "source /home/vagrant/andsdk-env.sh && /gamecake/build/install --android"


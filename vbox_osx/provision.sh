
echo " updating brew "
su vagrant -c " brew update "
su vagrant -c " git -C /usr/local/Homebrew/Library/Taps/homebrew/homebrew-core fetch --unshallow "
su vagrant -c " brew update --auto-update "


echo " installing bash "
su vagrant -c " brew install bash "
echo /usr/local/bin/bash | sudo tee -a /private/etc/shells
sudo chpass -s /usr/local/bin/bash vagrant


echo " fetching gamecake "
su vagrant -c " cd gamecake && ./git-pull && cd .. || git clone --recursive -v --progress https://github.com/xriss/gamecake.git  "


#echo " installing gamecake dependencies "
#su vagrant -c " /usr/local/bin/bash -c \"  gamecake/build/install  \" "
 

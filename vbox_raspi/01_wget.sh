cd `dirname $0`

if [ -f 2015-02-18-wheezy-minibian.img ] ; then

	echo " 2015-02-18-wheezy-minibian.img exists so skipping download and unpack"

else

	wget -o minibian.img.tar.gz http://sourceforge.net/projects/minibian/files/2015-02-18-wheezy-m inibian.tar.gz/download
	tar xvfz minibian.img.tar.gz

fi

if [ -f kernel-qemu ] ; then

	echo " kernel-qemu exists so skipping download "

else

	wget https://github.com/dhruvvyas90/qemu-rpi-kernel/raw/master/kernel-qemu

fi

echo " installing qemu "

sudo apt-get install qemu-system

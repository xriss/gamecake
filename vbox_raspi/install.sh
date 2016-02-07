cd `dirname $0`
THISDIR=`dirname $0`


#update these to get a newer version
RASPBIAN_FILE=2015-11-21-raspbian-jessie-lite
RASPBIAN_URL=https://downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2015-11-24/$RASPBIAN_FILE.zip


if [ -f raspbian.img ] ; then

	echo " raspbian.img exists so skipping download and unpack "

else

	wget -O $RASPBIAN_FILE.zip $RASPBIAN_URL
	unzip -o $RASPBIAN_FILE.zip
	rm $RASPBIAN_FILE.zip
	mv $RASPBIAN_FILE.img raspbian.img

fi




if [ -f kernel-qemu ] ; then

	echo " kernel-qemu exists so skipping download "

else

	wget -O kernel-qemu https://github.com/polaco1782/raspberry-qemu/blob/master/kernel-qemu?raw=true

fi

echo " installing qemu "

sudo apt-get install qemu-system sshpass

echo " copying raspbian "
cp raspbian.img raspi.img



echo " resizing to 3gig "
qemu-img resize -f raw raspi.img 3G

echo " checking partition information "

PART_BOOT_START=$(parted raspi.img -ms unit s print | grep "^1" | cut -f 2 -d: | cut -f 1 -ds)
PART_ROOT_START=$(parted raspi.img -ms unit s print | grep "^2" | cut -f 2 -d: | cut -f 1 -ds)
echo $PART_BOOT_START $PART_ROOT_START

echo " resizing using fdisk "
fdisk raspi.img <<EOF
p
d
2
n
p
2
$PART_ROOT_START

p
w
EOF

#these config changes are needed to allow qemu to boot
# we mount the image locally then make the changes then umount
#the image will no longer boot as an sdcard

./box-mount

sudo tee root/etc/ld.so.preload >/dev/null <<EOF

#/usr/lib/arm-linux-gnueabihf/libarmmem.so

EOF

sudo tee root/etc/fstab >/dev/null <<EOF

proc       /proc           proc    defaults          0       0
/dev/sda2  /               ext4    defaults,noatime  0       1

EOF

./box-umount



echo "starting qemu but detaching it from this shell, wait until we can login before running the next script"
./box-up >/dev/null &

echo " copying your id_rsa.pub to the PIs authorised keys for auto login "
ssh-keygen -f "$HOME/.ssh/known_hosts" -R [localhost]:5522
while ! cat ~/.ssh/id_rsa.pub | sshpass -p raspberry ssh -oStrictHostKeyChecking=no -p 5522 pi@localhost " mkdir -p .ssh ; cat >> .ssh/authorized_keys "
do
	sleep 1
    echo "Trying ssh login again..."
done

echo " apply final resize of partition "
./ssh " sudo resize2fs /dev/sda2 "

echo " updating apt info and sites"
./ssh " sudo apt-get -y update "
./ssh " sudo apt-get -y upgrade "

echo " installing packages we will need for building etc"
./ssh " sudo apt-get -y install aptitude sudo nano byobu pkg-config raspi-config lynx curl git mercurial gcc build-essential cmake "



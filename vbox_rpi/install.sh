cd `dirname $0`
THISDIR=`dirname $0`

echo " installing qemu "

./install-host-dependencies.sh

mkdir -p roms

#update these to get a newer version
RASPBIAN_FILE=2024-11-19-raspios-bookworm-armhf-lite
RASPBIAN_URL=https://downloads.raspberrypi.org/raspios_lite_armhf/images/raspios_lite_armhf-2024-11-19/$RASPBIAN_FILE.img.xz

if [ -f roms/raspbian.img ] ; then

	echo " raspbian.img exists so skipping download and unpack "

else

	wget -O roms/$RASPBIAN_FILE.img.xz $RASPBIAN_URL
	unxz roms/$RASPBIAN_FILE.img.xz
	mv roms/$RASPBIAN_FILE.img roms/raspbian.img

fi


echo " copying raspbian "
cp roms/raspbian.img roms/raspi.img



echo " resizing to 8gig "
qemu-img resize -f raw roms/raspi.img 8G

echo " checking partition information "

PART_BOOT_START=$(parted roms/raspi.img -ms unit s print | grep "^1" | cut -f 2 -d: | cut -f 1 -ds)
PART_ROOT_START=$(parted roms/raspi.img -ms unit s print | grep "^2" | cut -f 2 -d: | cut -f 1 -ds)
echo $PART_BOOT_START $PART_ROOT_START

echo " resizing using fdisk "
fdisk roms/raspi.img <<EOF
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

sudo tee root/boot/ssh boot/ssh <<EOF
ENABLE SSH
EOF
#undo the removal of the default login so we can use pi:raspberry again...
sudo tee root/boot/userconf boot/userconf <<'EOF'
pi:$6$rG.YX9cuM9xHE9nt$1zuuFRzSuXrEqSww2Wn7ZR.CtOsm9BIh9XfHwO.7a8sEL.QZVM2SedPJfFLTqwJcyxBHXzVA80szKuTfgDejZ1
EOF

# disable security rename user request
sudo rm root/etc/ssh/sshd_config.d/rename_user.conf

echo " copying kernel and dtb "
cp boot/bcm2710-rpi-3-b-plus.dtb roms/bcm2710-rpi-3-b-plus.dtb
cp boot/kernel8.img roms/kernel8.img

./box-umount



echo "starting qemu but detaching it from this shell, wait until we can login before running the next script"
./box-up >/dev/null &

echo " copying your id_rsa.pub to the PIs authorised keys for auto login "
ssh-keygen -f "$HOME/.ssh/known_hosts" -R [localhost]:5522
while ! cat ~/.ssh/id_rsa.pub | sshpass -p raspberry ssh -oStrictHostKeyChecking=no -p 5522 pi@localhost " mkdir -p .ssh ; cat >> .ssh/authorized_keys ; sudo systemctl enable ssh "
do
	sleep 10
    echo "Trying ssh login again..."
done

#echo " apply final resize of partition "
#./ssh " sudo resize2fs /dev/sda2 "

echo " updating apt info and sites"
./ssh " sudo apt-get update -y "
#./ssh " sudo apt-get upgrade -y "

echo " installing packages we will need for building etc"
./ssh " sudo apt-get install -y aptitude "
./ssh " sudo apt-get install -y sudo "
./ssh " sudo apt-get install -y nano "
./ssh " sudo apt-get install -y byobu "
./ssh " sudo apt-get install -y pkg-config "
./ssh " sudo apt-get install -y raspi-config "
#./ssh " sudo apt-get install -y lynx "
./ssh " sudo apt-get install -y curl "
./ssh " sudo apt-get install -y git "
#./ssh " sudo apt-get install -y mercurial "
./ssh " sudo apt-get install -y gcc "
./ssh " sudo apt-get install -y build-essential "
#./ssh " sudo apt-get install -y cmake "
#./ssh " sudo apt-get install -y npm "

echo " installing build dependencies"
./ssh " sudo apt-get install -y libssl-dev "
./ssh " sudo apt-get install -y libpulse-dev "
./ssh " sudo apt-get install -y libasound2-dev "

./ssh " sudo apt-get install -y lua5.1 "
./ssh " sudo apt-get install -y lua-filesystem "

./ssh " sudo apt-get install -y libsdl2-dev "

./ssh " sudo apt-get install -y libluajit-5.1-dev "


./ssh " sudo apt-get install -y libarchive-tools "


./box-down

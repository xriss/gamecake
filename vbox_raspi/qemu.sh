
qemu-system-arm -kernel kernel-qemu -cpu arm1176 -m 256 -M versatilepb -no-reboot -append "root=/dev/sda2 panic=1 console=ttyAMA0  console=ttyS0" -hda 2015-02-18-wheezy-minibian.img -redir tcp:5022::22 -nographic


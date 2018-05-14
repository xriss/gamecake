sudo usb_modeswitch -W -v 12d1 -p 1030 -V 12d1 -P 1034 -M "5553424370ab71890600000080010a11060000000000000000000000000000" -s 20
sudo ./adb kill-server
sudo ./adb start-server
sudo ./adb devices
sudo ./adb -d logcat


project "lib_usb"
language "C"
includedirs { "git/libusb" , "." }
files { "git/libusb/*.c" , "git/libusb/*.h" }



files {
	"git/libusb/os/events_posix.h" ,
	"git/libusb/os/events_posix.c" ,
	"git/libusb/os/threads_posix.h" ,
	"git/libusb/os/threads_posix.c" ,
	"git/libusb/os/linux_netlink.c" ,
	"git/libusb/os/linux_udev.c" ,
	"git/libusb/os/linux_usbfs.c" ,
	"git/libusb/os/linux_usbfs.h" ,
}
 
--wasm
--files { "git/libusb/os/emscripten_webusb.cpp" }


KIND{}

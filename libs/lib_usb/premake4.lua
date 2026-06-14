
project "lib_usb"
language "C"
includedirs { "git/libusb" }
files { "git/libusb/*.c" , "git/libusb/*.h" }



if WINDOWS then

defines { "PLATFORM_WINDOWS" }

includedirs { "os/windows" }

files {
	"git/libusb/os/events_windows.h" ,
	"git/libusb/os/events_windows.c" ,
	"git/libusb/os/threads_windows.h" ,
	"git/libusb/os/threads_windows.c" ,
	"git/libusb/os/windows_common.c" ,
	"git/libusb/os/windows_common.h" ,
	"git/libusb/os/windows_hotplug.c" ,
	"git/libusb/os/windows_hotplug.h" ,
	"git/libusb/os/windows_usbdk.c" ,
	"git/libusb/os/windows_usbdk.h" ,
	"git/libusb/os/windows_winusb.c" ,
	"git/libusb/os/windows_winusb.h" ,
}

else

defines { "PLATFORM_LINUX" }

includedirs { "os/linux"}

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

end
 
--wasm
--files { "git/libusb/os/emscripten_webusb.cpp" }


KIND{}

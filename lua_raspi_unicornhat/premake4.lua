project "lua_raspi_unicornhat"
language "C"


-- the  library

includedirs { "unicornhat/python/rpi-ws281x/lib/" }
files {

	"unicornhat/python/rpi-ws281x/lib/board_info.c",
	"unicornhat/python/rpi-ws281x/lib/dma.c",
	"unicornhat/python/rpi-ws281x/lib/mailbox.c",
	"unicornhat/python/rpi-ws281x/lib/pwm.c",
	"unicornhat/python/rpi-ws281x/lib/ws2811.c",

}


-- bindings

files { "code/*.c" }
includedirs { "code" }


KIND{lua="raspi.unicornhat"}

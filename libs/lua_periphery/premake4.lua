
project "lua_periphery"

language "C"

includedirs {
	".",
	"./c-periphery/src/",
}

files {
	"./src/lua_gpio.c",
	"./src/lua_i2c.c",
	"./src/lua_mmio.c",
	"./src/lua_serial.c",
	"./src/lua_spi.c",
	"./src/lua_periphery.c",
}

KIND{kind="lua",name="periphery"}

include( "c-periphery" ) 



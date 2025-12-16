
project "lib_periphery"

language "C"

includedirs {
	".",
	"./src/",
}

files {
	"./src/gpio.c",
	"./src/i2c.c",
	"./src/mmio.c",
	"./src/serial.c",
	"./src/spi.c",
}

KIND{}



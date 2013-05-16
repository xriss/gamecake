
project "lib_hidapi"
language "C"

includedirs { "hidapi" }

if WINDOWS then

files {
	"windows/hid.c",
}

else

files {
	"linux/hid.c",
}

end

KIND{}


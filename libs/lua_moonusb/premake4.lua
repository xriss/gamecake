
project "lua_moonusb"
language "C"
includedirs { "git/src" }
files { "git/src/**.c" , "git/src/**.h" }
links { "lib_lua" }

defines { "COMPAT53_PREFIX=moonusb_compat_" }

if WINDOWS then
--	defines { "MINGW" }
	defines { "LINUX" }
else
	defines { "LINUX" }
end

KIND{lua="moonusb"}

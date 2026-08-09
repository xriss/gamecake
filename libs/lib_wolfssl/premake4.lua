
project "lib_wolfssl"
language "C"


includedirs { ".", "git", }

files{ "git/src/**.c" }

files{ "git/wolfcrypt/src/**.c" }

excludes{
	"git/src/conf*",
	"git/src/bio*",
	"git/src/x509*",
	"git/src/ssl_*",
	"git/src/pk*",
	"git/wolfcrypt/src/asn_*",
	"git/wolfcrypt/src/evp*",
	"git/wolfcrypt/src/misc*",
}

defines{
-- put it all in user_settings.h
	"WOLFSSL_USER_SETTINGS",
}

	buildlinkoptions{
		"-Wno-implicit-function-declaration",
	}



if WINDOWS then

--	defines "USE_WINDOWS_API"
	defines "_WIN32_WINNT=0x0600"
--	defines "WINVER=0x0600"

	links { "ws2_32" }

end

configuration "Debug"
	defines{
		"DEBUG_WOLFSSL",
		"WOLFSSL_LOGGINGENABLED_DEFAULT=1",
	}
configuration {}

KIND{}


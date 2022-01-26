
project "lib_wolfssl"
language "C"


includedirs { ".", "git", }

files{
	"git/src/**.c",
	"git/wolfcrypt/src/**.c",
}

defines{
	"HAVE_EX_DATA",
	"SESSION_CERTS",
	"HAVE_ALPN",
	"WOLFSSL_ALLOW_TLSV10",
	"WOLFSSL_CERT_GEN",
	"WOLFSSL_USER_SETTINGS",
	"OPENSSL_EXTRA",
	"OPENSSL_ALL",
	"WOLFSSL_ALT_CERT_CHAINS",
--	"DEBUG_WOLFSSL",
}

	buildlinkoptions{
		"-Wno-implicit-function-declaration",
	}



if WINDOWS then

	defines "USE_WINDOWS_API"
	
	defines "_WIN32_WINNT=0x0600"

	links { "ws2_32" }

end


KIND{}



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

KIND{}


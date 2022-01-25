
project "lib_wolfssl"
language "C"


includedirs { "git", }

files{
	"git/src/**.c",
	"git/wolfcrypt/src/**.c",
}

defines{
	"OPENSSL_EXTRA",
	"OPENSSL_ALL",
	"HAVE_EX_DATA",
	"HAVE_TLS_EXTENSIONS",
	"HAVE_AESGCM",
	"HAVE_ALPN",
	"SESSION_CERTS",
	"WOLFSSL_CERT_GEN",
	"WOLFSSL_ALLOW_TLSV10",
	"WOLFSSL_SHA512",
}

KIND{}


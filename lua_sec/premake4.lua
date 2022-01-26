
local lua_sec_all=function()

includedirs{
	"git/src",
	"../lib_wolfssl/git",
	"../lib_wolfssl/git/wolfssl",
}

defines{
	"OPENSSL_EXTRA",
	"OPENSSL_ALL",
	"HAVE_DH",
	"HAVE_EX_DATA",
	"HAVE_TLS_EXTENSIONS",
	"HAVE_AESGCM",
	"HAVE_ALPN",
	"HAVE_SHA512",
--	"HAVE_NULL_CIPHER",
	"SESSION_CERTS",
	"WOLFSSL_CERT_GEN",
	"WOLFSSL_ALLOW_TLSV10",
	"WOLFSSL_TLS12",
	"WOLFSSL_SHA512",
	"WOLFSSL_SHA384",
	"WOLFSSL_TLS13",
	"WC_RSA_PSS",
	"HAVE_FFDHE_2048",
	"HAVE_FFDHE_3072",
	"HAVE_FFDHE_4096",
	"HAVE_FFDHE_6144",
	"HAVE_FFDHE_8192",
	"HAVE_HKDF",
	"HAVE_SUPPORTED_CURVES",
	"WOLFSSL_ALT_CERT_CHAINS",
}



links { "lib_wolfssl" }

end


project "lua_sec_core"
language "C"

lua_sec_all()

files { "git/src/ssl.*" }

KIND{lua="ssl.core"}



project "lua_sec_context"
language "C"

lua_sec_all()

files { "git/src/context.*" }
files { "git/src/options.*" }
files { "git/src/ec.*" }

KIND{lua="ssl.context"}



project "lua_sec_x509"
language "C"

lua_sec_all()

files { "git/src/x509.*" }

KIND{lua="ssl.x509"}



project "lua_sec_config"
language "C"

lua_sec_all()

files { "git/src/config.*" }

KIND{lua="ssl.config"}


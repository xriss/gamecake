
local lua_sec_all=function()

includedirs{
	"git/src",
	"../lib_wolfssl/git",
	"../lib_wolfssl/git/wolfssl",
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


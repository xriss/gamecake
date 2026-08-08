
local lua_sec_all=function()

includedirs{
	"git/src",
	"../lib_wolfssl",
	"../lib_wolfssl/git",
	"../lib_wolfssl/git/wolfssl",
}

-- wolfssl
defines{
-- put it all in user_settings.h
	"WOLFSSL_USER_SETTINGS",
}

defines{
	"LSEC_API_OPENSSL_1_1_0",
--	"TLS1_3_VERSION",
}

if WINDOWS then

	defines "LUASEC_INET_NTOP"

end

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


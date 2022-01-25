

project "lua_sec_core"
language "C"

includedirs { "git/src" }

files { "git/src/ssl.*" }

links { "lua_socket" }
links { "lib_openssl" }

KIND{lua="ssl.core"}



project "lua_sec_context"
language "C"

includedirs { "git/src" }

files { "git/src/context.*" }

links { "lua_socket" }
links { "lib_openssl" }

KIND{lua="ssl.context"}



project "lua_sec_x509"
language "C"

includedirs { "git/src" }

files { "git/src/x509.*" }

links { "lua_socket" }
links { "lib_openssl" }

KIND{lua="ssl.x509"}



project "lua_sec_config"
language "C"

includedirs { "git/src" }

files { "git/src/config.*" }

links { "lua_socket" }
links { "lib_openssl" }

KIND{lua="ssl.config"}


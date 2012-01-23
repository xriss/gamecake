

project "lua_mysql"
language "C++"
files { "src/luasql.*" , "src/ls_mysql.c" }

links { "lib_lua" }

if WINDOWS then

	includedirs { "../windows/mysql51/include" }
	libdirs { "../windows/mysql51/lib" }

	defines "LUASQL_API=__declspec(dllexport)"
	
	links "libmySQL"

else -- nix

	includedirs { "/usr/include/mysql" }
	links { "mysqlclient" }

end

SET_KIND("lua","luasql.mysql","luasql_mysql")
SET_TARGET("/luasql","msql")


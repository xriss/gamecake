
project "lib_pq"
kind "StaticLib"
language "C"
files {
	"./src/interfaces/libpq/fe-auth.c",
	"./src/interfaces/libpq/fe-connect.c",
	"./src/interfaces/libpq/fe-exec.c",
	"./src/interfaces/libpq/fe-misc.c",
	"./src/interfaces/libpq/fe-print.c",
	"./src/interfaces/libpq/fe-lobj.c",
	"./src/interfaces/libpq/fe-protocol2.c",
	"./src/interfaces/libpq/fe-protocol3.c",
	"./src/interfaces/libpq/pqexpbuffer.c",
	"./src/interfaces/libpq/fe-secure.c",
	"./src/interfaces/libpq/libpq-events.c",	
	
	"./src/interfaces/libpq/chklocale.c",	
	"./src/interfaces/libpq/inet_net_ntop.c",	
	"./src/interfaces/libpq/noblock.c",	
	"./src/interfaces/libpq/pgstrcasecmp.c",	
	"./src/interfaces/libpq/pqsignal.c",	
	"./src/interfaces/libpq/thread.c",	

--	"./src/interfaces/libpq/crypt.c",	
--	"./src/interfaces/libpq/getaddrinfo.c",	
	"./src/interfaces/libpq/getpeereid.c",	
--	"./src/interfaces/libpq/inet_aton.c",	
--	"./src/interfaces/libpq/open.c",	
--	"./src/interfaces/libpq/system.c",	
--	"./src/interfaces/libpq/snprintf.c",	
--	"./src/interfaces/libpq/strerror.c",	
	"./src/interfaces/libpq/strlcpy.c",	
--	"./src/interfaces/libpq/win32error.c",	
--	"./src/interfaces/libpq/win32setlocale.c",	


	"./src/interfaces/libpq/ip.c",	
	"./src/interfaces/libpq/md5.c",	

	"./src/interfaces/libpq/encnames.c",	
	"./src/interfaces/libpq/wchar.c",	
	}


-- libpq is now half bullshit and relies on makefile generated tat
-- wire protocol my arse so we stick with this old junk for now
buildoptions { "-std=gnu17" }

defines{ "FRONTEND" , "UNSAFE_STAT_OK" }

includedirs { "." , "./src/include" , "./src/interfaces/libpq" , "./src/port" , "./src/backend" }


KIND{}


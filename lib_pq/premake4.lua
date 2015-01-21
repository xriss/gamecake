
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
--[[
# We can't use Makefile variables here because the MSVC build system scrapes
# OBJS from this file.
OBJS=	fe-auth.o fe-connect.o fe-exec.o fe-misc.o fe-print.o fe-lobj.o \
	fe-protocol2.o fe-protocol3.o pqexpbuffer.o fe-secure.o \
	libpq-events.o
# libpgport C files we always use
OBJS += chklocale.o inet_net_ntop.o noblock.o pgstrcasecmp.o pqsignal.o \
	thread.o
# libpgport C files that are needed if identified by configure
OBJS += $(filter crypt.o getaddrinfo.o getpeereid.o inet_aton.o open.o system.o snprintf.o strerror.o strlcpy.o win32error.o win32setlocale.o, $(LIBOBJS))
# backend/libpq
OBJS += ip.o md5.o
# utils/mb
OBJS += encnames.o wchar.o
]]

defines{ "FRONTEND" , "UNSAFE_STAT_OK" }

includedirs { "." , "./src/include" , "./src/interfaces/libpq" , "./src/port" , "./src/backend" }


KIND{}


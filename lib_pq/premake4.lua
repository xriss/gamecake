
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



includedirs { "." , "./src/include" , "./src/interfaces/libpq"}


KIND{}


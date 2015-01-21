
project "lib_pq"
kind "StaticLib"
language "C"
files {
	"src/interface/libpq/fe-auth.c",
	"src/interface/libpq/fe-connect.c",
	"src/interface/libpq/fe-exec.c",
	"src/interface/libpq/fe-misc.c",
	"src/interface/libpq/fe-print.c",
	"src/interface/libpq/fe-lobj.c",
	"src/interface/libpq/fe-protocol2.c",
	"src/interface/libpq/fe-protocol3.c",
	"src/interface/libpq/pqexpbuffer.c",
	"src/interface/libpq/fe-secure.c",
	"src/interface/libpq/libpq-events.c",	
	
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



includedirs { "." }


KIND{}



project "lib_pcre"
kind "StaticLib"
language "C"
files {
	"pcre_compile.c",
	"pcre_config.c",
	"pcre_dfa_exec.c",
	"pcre_exec.c",
	"pcre_fullinfo.c",
	"pcre_get.c",
	"pcre_globals.c",
	"pcre_info.c",
	"pcre_internal.h",
	"pcre_maketables.c",
	"pcre_newline.c",
	"pcre_ord2utf8.c",
	"pcre_refcount.c",
	"pcre_study.c",
	"pcre_tables.c",
	"pcre_try_flipped.c",
	"pcre_ucd.c",
	"pcre_valid_utf8.c",
	"pcre_version.c",
	"pcre_xclass.c",
	"ucp.h",
	"dftables.out.c",
 }

--, "LINK_SIZE=2" ,
defines { "HAVE_CONFIG_H", "PCRE_STATIC" , "POSIX_MALLOC_THRESHOLD=10" , "SUPPORT_PCRE8" }

includedirs { "." }

KIND{}


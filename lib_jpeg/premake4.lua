
project "lib_jpeg"
kind "StaticLib"
language "C"
files {  "./**.c" , "./**.h" }
excludes {
	"./jmemdos.c" ,
	"./jmemmac.c" ,
	"./ansi2knr.c" ,
	"./example.c" ,
	"./cdjpeg.c" , 
	"./cjpeg.c", 
	"./djpeg.c" , 
	"./rdjpgcom.c" , 
	"./wrjpgcom.c" , 
	"./ckconfig.c" , 
	"./jpegtran.c" ,
}

defines { "JPEGSTATIC" , "NO_GETENV" }

includedirs { "." }

KIND{}



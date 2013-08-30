
project "lib_jpeg"
kind "StaticLib"
language "C"
files {  "jpeg-6b/**.c" , "jpeg-6b/**.h" }
excludes {
	"jpeg-6b/jmemdos.c" ,
	"jpeg-6b/jmemmac.c" ,
	"jpeg-6b/ansi2knr.c" ,
	"jpeg-6b/example.c" ,
	"jpeg-6b/cdjpeg.c" , 
	"jpeg-6b/cjpeg.c", 
	"jpeg-6b/djpeg.c" , 
	"jpeg-6b/rdjpgcom.c" , 
	"jpeg-6b/wrjpgcom.c" , 
	"jpeg-6b/ckconfig.c" , 
	"jpeg-6b/jpegtran.c" ,
}

defines { "JPEGSTATIC" }


includedirs { "." }

KIND{}



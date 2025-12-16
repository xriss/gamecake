
project "lib_jpeg"
kind "StaticLib"
language "C"

jpegroot="./jpeg"

files {  jpegroot.."/**.c" , jpegroot.."/**.h" }
excludes {
	jpegroot.."/jmemdos.c" ,
	jpegroot.."/jmemmac.c" ,
	jpegroot.."/jmemansi.c" ,
	jpegroot.."/jmemname.c" ,
	jpegroot.."/ansi2knr.c" ,
	jpegroot.."/example.c" ,
	jpegroot.."/cdjpeg.c" , 
	jpegroot.."/cjpeg.c", 
	jpegroot.."/djpeg.c" , 
	jpegroot.."/rdjpgcom.c" , 
	jpegroot.."/wrjpgcom.c" , 
	jpegroot.."/ckconfig.c" , 
	jpegroot.."/jpegtran.c" ,
}

defines { "JPEGSTATIC" , "NO_GETENV" }

-- this fixes the windows build without changing the config.h
defines { "TRUE=1", "FALSE=0"}

includedirs { jpegroot }

KIND{}



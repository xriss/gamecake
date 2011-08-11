

project "lua_wx"
language "C++"

includedirs { "wx/include"  }



for i,v in ipairs{

--"ansi2knr.1",
"ansi2knr.c",
--"change.log",
"jcapimin.c",
"jcapistd.c",
"jccoefct.c",
"jccolor.c",
"jcdctmgr.c",
"jchuff.c",
"jchuff.h",
"jcinit.c",
"jcmainct.c",
"jcmarker.c",
"jcmaster.c",
"jcomapi.c",
"jconfig.h",
--"jconfig.vc",
"jcparam.c",
"jcphuff.c",
"jcprepct.c",
"jcsample.c",
"jctrans.c",
"jdapimin.c",
"jdapistd.c",
"jdatadst.c",
"jdatasrc.c",
"jdcoefct.c",
"jdcolor.c",
"jdct.h",
"jddctmgr.c",
"jdhuff.c",
"jdhuff.h",
"jdinput.c",
"jdmainct.c",
"jdmarker.c",
"jdmaster.c",
"jdmerge.c",
"jdphuff.c",
"jdpostct.c",
"jdsample.c",
"jdtrans.c",
"jerror.c",
"jerror.h",
"jfdctflt.c",
"jfdctfst.c",
"jfdctint.c",
"jidctflt.c",
"jidctfst.c",
"jidctint.c",
"jidctred.c",
"jinclude.h",
"jmemansi.c",
--"jmemdos.c",
--"jmemdosa.asm",
--"jmemmac.c",
"jmemmgr.c",
"jmemname.c",
"jmemnobs.c",
"jmemsys.h",
"jmorecfg.h",
"jpeg.dsp",
"jpeg_CW_Prefix.h",
"jpegint.h",
"jpeglib.h",
"jpegtran.1",
"jpegtran.c",
"jquant1.c",
"jquant2.c",
"jutils.c",
"jversion.h",

}do
	files { "wx/src/jpeg/"..v }

end

for i,v in ipairs{
--"jpeg",
"png", 
"tiff", 
"zlib", 
"regex", 
"expat",
--"base", 
--"net", 
--"odbc", 
--"core", 
--"gl", 
"html", 
"xml", 
--"media", 
--"qa", 
--"adv", 
--"dbgrid", 
"xrc", 
"aui",
"richtext",
} do

	files { "wx/src/"..v.."/*.h" , "wx/src/"..v.."/*.c" , "wx/src/"..v.."/*.cpp" }

end

links { "lua51" }

if os.get() == "windows" then

--	files { "iup/src/win/*.h" , "iup/src/win/*.c" }

else -- nix

--	files { "iup/src/gtk/iupgtk*.h" , "iup/src/gtk/iupgtk*.c" }

	local fp=assert(io.popen("pkg-config --cflags gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	buildoptions { s }
	fp:close()

	local fp=assert(io.popen("pkg-config --libs gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	linkoptions { s }
	fp:close()
end


-- files { "iup/srclua5/*.h" , "iup/srclua5/*.c" }


SET_KIND("lua","lua_wx","wx")
SET_TARGET("","lua_wx")


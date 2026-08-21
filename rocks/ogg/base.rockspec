build={
 modules={
      ["wetgenes.ogg"]="lua_ogg/code/ogg.lua",
      ["wetgenes.ogg.core"]={
         sources={
            "lua_ogg/code/lua_ogg.c",
         },
         incdirs={
            "lua_ogg",
            "lib_hacks/code",
            "$(VORBIS_LIB_INCDIR)",
            "$(OGG_LIB_INCDIR)",
         },
		libdirs={
			"$(VORBIS_LIB_LIBDIR)",
			"$(OGG_LIB_LIBDIR)",
		},
         libraries = {"vorbis","ogg"},
      },
 },
 platform={
  windows={
  },
 },
 type="builtin",
}

external_dependencies = {
   VORBIS_LIB = {
      header = "vorbis/codec.h",
   },
   OGG_LIB = {
      header = "ogg/ogg.h",
   },
}


dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="read ogg vorbis audio files",
 detailed=[[

Needs libvorbis and libogg pre installed.

See libs/lua_ogg/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_ogg/

 ]],
}

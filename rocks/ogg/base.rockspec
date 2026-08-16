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
         },
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
}


dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.wetgenes/",
 license="MIT",
 summary="read ogg vorbis audio files",
 detailed=[[

https://github.com/xriss/gamecake

 ]],
}

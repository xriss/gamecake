build={
 modules={
--      ["wetgenes.opus"]="lua_opus/code/opus.lua",
      ["wetgenes.opus.core"]={
         sources={
            "lua_opus/code/lua_opus.c",
            "lua_pack/code/lua_pack.c",
         },
         incdirs={
            "lua_opus",
            "lib_hacks/code",
            "lua_hacks/code",
            "$(OPUS_LIB_INCDIR)/opus",
            "$(SPEEX_LIB_INCDIR)/speex",
         },
         libdirs={
            "$(OPUS_LIB_LIBDIR)",
            "$(SPEEX_LIB_LIBDIR)",
         },
         libraries = {"opus","speex"},
      },
 },
 platform={
  windows={
  },
 },
 type="builtin",
}
external_dependencies = {
   OPUS_LIB = {
      header = "opus/opus.h",
   },
   SPEEX_LIB = {
      header = "speex/speex_echo.h",
   },
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.wetgenes/",
 license="MIT",
 summary="read opus audio files",
 detailed=[[

https://github.com/xriss/gamecake

 ]],
}

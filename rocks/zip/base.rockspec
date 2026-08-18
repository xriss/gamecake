build={
 modules={
  ["wetgenes.zip"]={
   incdirs={
    "lua_zip/src",
    "lib_hacks/code",
    "$(ZZIP_INCDIR)"
   },
   sources={
    "lua_zip/src/luazip.c",
   },
   libdirs = {
    "$(ZZIP_LIBDIR)"
   },
   libraries = {
    "zzip"
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
   ZZIP = {
      header = "zzip.h"
   }
}dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.wetgenes/",
 license="MIT",
 summary="modified version of luazip",
 detailed=[[

Sometime We need to be able to load zips from memory, not the filesystem.

 ]],
}

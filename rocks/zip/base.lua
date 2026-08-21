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
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="modified version of luazip",
 detailed=[[

We need to be able to load zips from memory, not the filesystem so this 
is a slightly modified version of luazip with that functionality added.

See libs/lua_zip/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_zip/

 ]],
}

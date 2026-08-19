build={
 modules={
  ["wetgenes.al"]="lua_al/code/al.lua",
  ["wetgenes.al.core"]={
   incdirs={
    "lua_al",
    "lib_hacks/code",
    "$(AL_LIB_INCDIR)",
   },
   sources={
    "lua_al/code/lua_al.c",
   },
   defines={
   },
   libdirs={
    "$(AL_LIB_LIBDIR)",
   },
   libraries = {"openal"},
  },
  ["wetgenes.alc"]="lua_al/code/alc.lua",
  ["wetgenes.alc.core"]={
   incdirs={
    "lua_al",
    "lib_hacks/code",
    "$(AL_LIB_INCDIR)",
   },
   sources={
    "lua_al/code/lua_alc.c",
   },
   defines={
   },
   libdirs={
    "$(AL_LIB_LIBDIR)",
   },
   libraries = {"openal"},
  },
 },
 platform={
  windows={
  },
 },
 type="builtin",
}
external_dependencies = {
   AL_LIB = {
      header = "AL/al.h"
   },
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.al/",
 license="MIT",
 summary="OpenAL",
 detailed=[[

OpenAL binding, provides al and alc modules.


 ]],
}

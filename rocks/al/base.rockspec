build={
 modules={
  al="lua_al/code/al.lua",
  ["al.core"]={
   incdirs={
    "lib_openal/mojoal",
    "lib_openal/mojoal/AL",
    "lua_al",
    "lib_hacks/code",
    "$(SDL_LIB_INCDIR)/SDL2",
   },
   sources={
    "lua_al/code/lua_al.c",
    "lib_openal/mojoal/mojoal.c",
   },
   defines={
   },
   libdirs={
    "$(SDL_LIB_LIBDIR)",
   },
   libraries = {"SDL2"},
  },
  alc="lua_al/code/alc.lua",
  ["alc.core"]={
   incdirs={
    "lib_openal/mojoal",
    "lib_openal/mojoal/AL",
    "lua_al",
    "lib_hacks/code",
    "$(SDL_LIB_INCDIR)/SDL2",
   },
   sources={
    "lua_al/code/lua_alc.c",
    "lib_openal/mojoal/mojoal.c",
   },
   defines={
   },
   libdirs={
    "$(SDL_LIB_LIBDIR)",
   },
   libraries = {"SDL2"},
  },
 },
 platform={
  windows={
   ["wire.core"]={
    defines={
    },
   },
  },
 },
 type="builtin",
}
external_dependencies = {
   SDL_LIB = {
      header = "SDL2/SDL_scancode.h"
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

OpenAL is via mojoal which uses SDL for the actual audio interface.

https://github.com/icculus/mojoAL/

This works anywhere SDL works, so phones/web/pc etc.

 ]],
}

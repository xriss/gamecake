build={
 modules={
      ["wetgenes.win"]="lua_win/code/win.lua",
      ["wetgenes.win.core"]={
         sources={
            "lua_win/code/lua_win.c",
         },
         incdirs={
            "lua_win",
            "lib_hacks/code",
			"$(SDL_LIB_INCDIR)/SDL2",
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
   SDL_LIB = {
      header = "SDL2/SDL_scancode.h"
   },
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.wetgenes/",
 license="MIT",
 summary="win hacks",
 detailed=[[

C hacks into windows / sdl to help manage windows.

 ]],
}

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
		 libraries = {"SDL2"},
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
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="win hacks",
 detailed=[[

C hacks into windows / sdl to help manage windows.

See libs/lua_win/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_win/


 ]],
}

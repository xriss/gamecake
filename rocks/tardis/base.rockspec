build={
 type="builtin",
 modules={

      ["wetgenes.tardis"]     = "lua_tardis/code/tardis.lua",
      ["wetgenes.tardis.core"]={
         sources={
            "lua_tardis/code/lua_tardis.c",
         },
         incdirs={
            "lua_tardis",
            "lib_hacks/code",
         },
      },

 },
 install = {
 },
 platform={
  windows={
  },
 },
}
external_dependencies = {
}
dependencies={
 "lua >= 5.1 <= 5.2", 
}
description={
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="Time and Relative Dimensions In Space",
 detailed=[[

Vector and Matrix maths.

See libs/lua_tardis/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_tardis/

 ]],
}


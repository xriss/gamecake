build={
 modules={
      ["wetgenes.dumbft"]="lua_dumbft/code/dumbft.lua",
      ["wetgenes.dumbft.core"]={
         sources={
            "lua_dumbft/code/lua_dumbft.c",
         },
         incdirs={
            "lua_dumbft/code",
            "lib_hacks/code",
         },
         libdirs={
         },
         libraries = {
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
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="dumbft sound processing",
 detailed=[[

Like FFT but dumber.

See libs/lua_dumbft/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_dumbft/

 ]],
}

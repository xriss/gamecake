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
 homepage="https://xriss.github.io/gamecake/docs/lua.wetgenes/",
 license="MIT",
 summary="dumbft sound processing",
 detailed=[[

https://github.com/xriss/gamecake

 ]],
}

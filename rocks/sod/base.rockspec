build={
 modules={
      ["wetgenes.sod"]="lua_sod/code/sod.lua",
      ["wetgenes.sod.core"]={
         sources={
            "lua_sod/code/lua_sod.c",
            "lua_sod/code/sod.c",
            "lua_sod/code/sod_wav.c",
         },
         incdirs={
            "lua_sod",
            "lib_hacks/code",
         },
      },
 },
 platform={
  windows={
  },
 },
 type="builtin",
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="Read wav audio files",
 detailed=[[

https://github.com/xriss/gamecake

See libs/lua_sod/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_sod/

 ]],
}

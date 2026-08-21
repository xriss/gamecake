build={
 modules={
      ["wetgenes.fats"]="lua_fats/code/fats.lua",
      ["wetgenes.fats.core"]={
         sources={
            "lua_fats/code/lua_fats.c",
         },
         incdirs={
            "lua_fats",
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
 summary="fat(int16_t,float,etc) binary strings",
 detailed=[[

Lua arrays of numbers, to and from strings made out of streams of ints 
or floats. Useful for building buffers for opengl (float) and audio 
(int16_t).

See libs/lua_fats/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_fats/

 ]],
}

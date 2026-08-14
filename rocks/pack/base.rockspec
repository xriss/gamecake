build={
 modules={
      ["wetgenes.pack"]="lua_pack/code/pack.lua",
      ["wetgenes.pack.core"]={
         sources={
            "lua_pack/code/lua_pack.c",
         },
         incdirs={
            "lua_pack",
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
 homepage="https://xriss.github.io/gamecake/docs/lua.pack/",
 license="MIT",
 summary="binary string packing",
 detailed=[[

ignore me

 ]],
}

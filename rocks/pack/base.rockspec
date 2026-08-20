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
 homepage="https://github.com/xriss/gamecake",
 license="MIT",
 summary="binary string userdata packing",
 detailed=[[

Lua library to help with read/write cdata style structs etc. This 
exists as we can not rely on luajits ffi structs always being available 
so needed a workaround for vanilla lua.

See libs/lua_pack/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_pack/

 ]],
}

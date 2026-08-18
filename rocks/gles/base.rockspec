build={
 modules={
  ["wetgenes.gles"]="lua_gles/code/gles.lua",
  ["wetgenes.glescode"]="lua_gles/code/glescode.lua",
  ["wetgenes.gles.core"]={
   incdirs={
    "lua_gles",
    "lua_gles/include",
    "lib_hacks/code",
   },
   sources={
    "lua_gles/code/lua_gles.c",
    "lua_gles/src/gl3w.c",
    "lua_tardis/code/lua_tardis.c",
   },
   defines={
    "INCLUDE_GLES_GL=\"GL/gl3w.h\"",
    "LUA_GLES_GL",
   },
  },
 },
 platform={
  windows={
   ["gles.core"]={
    defines={
     "HAVE_FCNTL_H=1",
    },
   },
  },
 },
 type="builtin",
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.gles/",
 license="MIT",
 summary="GLES",
 detailed=[[

GLES, so good for phone/web/desktop opengl lua binding.

Uses gl3w for maximum compatibility.

 ]],
}

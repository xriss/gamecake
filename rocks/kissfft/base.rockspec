build={
 modules={
--      ["wetgenes.kissfft"]="lua_kissfft/code/kissfft.lua",
      ["wetgenes.kissfft.core"]={
         sources={
            "lua_kissfft/code/lua_kissfft.c",
            "lua_kissfft/code/kiss_fftr.c",
            "lua_kissfft/code/kiss_fft.c",
         },
         incdirs={
            "lua_kissfft/code",
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
 summary="kissfft sound processing",
 detailed=[[

https://github.com/xriss/gamecake

 ]],
}

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
 homepage="https://github.com/xriss/gamecake",
 license="MIT / BSD-3-Clause",
 summary="kissfft sound processing",
 detailed=[[

A lua binding to kissfft https://github.com/mborgerding/kissfft

See libs/lua_kissfft/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_kissfft/

 ]],
}

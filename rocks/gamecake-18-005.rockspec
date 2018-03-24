package = "gamecake"
version = "18-005"
source = {
   url = "git@github.com:xriss/gamecake.git",
   tag = "V18.005"
}
description = {
   homepage = "http://wetgenes.com/",
   license = "MIT"
}
dependencies = {}
build = {
   type = "builtin",
   modules = {
      ["wetgenes.pack"]={
         sources={
            "lua_pack/code/lua_pack.c",
         },
         incdirs={
            "lua_pack"
         },
      },
   }
}

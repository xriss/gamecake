
package = "gamecake"

version = "18-005"

source = {
   url="src.zip",
}

description = {
   homepage = "http://wetgenes.com/",
   license = "MIT",
}

dependencies = {
   "bit32",
}

build = {
   type = "builtin",
   modules = {
      ["wetgenes.pack.core"]={
         sources={
            "lua_pack/code/lua_pack.c",
            "lib_hacks/code/hacks.c",
         },
         incdirs={
            "lua_pack",
            "lib_hacks/code",
         },
      },
      ["wetgenes.pack"]="lua_src/wetgenes/pack.lua",
      ["wetgenes.string"]="lua_src/wetgenes/string.lua",
   }
}

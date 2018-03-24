
package = "gamecake"

version = "18-005"

source = {
   url="src.zip",
}

description = {
   homepage = "http://wetgenes.com/",
   license = "MIT",
}

dependencies = {}

build = {
   type = "builtin",
   modules = {
      ["wetgenes.pack.core"]={
         sources={
            "lua_pack/code/lua_pack.c",
         },
         incdirs={
            "lua_pack"
         },
      },
      ["wetgenes.pack"]="lua_pack/code/pack.lua",
   }
}

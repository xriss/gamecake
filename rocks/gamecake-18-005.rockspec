
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

external_dependencies = {
   GIF_LIB = {
      header = "gif_lib.h"
   }
}

build = {
   type = "builtin",
   modules = {

      ["wetgenes.pack"]="lua_src/wetgenes/pack.lua",
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

      ["wetgenes.tardis"]="lua_src/wetgenes/tardis.lua",
      ["wetgenes.tardis.core"]={
         sources={
            "lua_tardis/code/lua_tardis.c",
            "lib_hacks/code/hacks.c",
         },
         incdirs={
            "lua_tardis",
            "lib_hacks/code",
         },
      },

      ["wetgenes.grd"]="lua_src/wetgenes/grd.lua",
      ["wetgenes.grd.core"]={
         sources={
            "lua_grd/code/lua_grd.c",
            "lua_grd/code/grd.c",
            "lua_grd/code/grd_gif.c",
            "lua_grd/code/grd_jpg.c",
            "lua_grd/code/grd_png.c",
            "lib_hacks/code/hacks.c",
         },
         incdirs={
            "lua_grd",
            "lib_hacks/code",
            "$(GIF_LIB_INCDIR)",
         },
         libdirs={
            "$(GIF_LIB_LIBDIR)",
         },
         libraries = {"gif_lib"},
      },

      ["wetgenes.string"]="lua_src/wetgenes/string.lua",
   }
}

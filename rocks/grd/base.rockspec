build={
 modules={

      ["wetgenes.grdcanvas"] ="lua_grd/code/grdcanvas.lua",
      ["wetgenes.grdhistory"]="lua_grd/code/grdhistory.lua",
      ["wetgenes.grdlayers"] ="lua_grd/code/grdlayers.lua",
      ["wetgenes.grdpaint"]  ="lua_grd/code/grdpaint.lua",
      ["wetgenes.grdsvg"]    ="lua_grd/code/grdsvg.lua",

      ["wetgenes.grd"]="lua_grd/code/grd.lua",
      ["wetgenes.grd.core"]={
         sources={
            "lua_grd/code/lua_grd.c",
            "lua_grd/code/grd.c",
            "lua_grd/code/grd_gif.c",
            "lua_grd/code/grd_jpg.c",
            "lua_grd/code/grd_png.c",
         },
         incdirs={
            "lua_grd",
            "lib_hacks/code",
            "$(PNG_LIB_INCDIR)",
            "$(GIF_LIB_INCDIR)",
            "$(JPEG_LIB_INCDIR)",
         },
         libdirs={
            "$(PNG_LIB_LIBDIR)",
            "$(GIF_LIB_LIBDIR)",
            "$(JPEG_LIB_LIBDIR)",
         },
         libraries = {"gif","png","jpeg"},
      },
      ["wetgenes.grdmap"]="lua_grdmap/code/grdmap.lua",
      ["wetgenes.grdmap.core"]={
         sources={
            "lua_grdmap/code/lua_grdmap.c",
            "lua_grdmap/code/grdmap.c",
            "lua_grdmap/code/grdmap_tiles.c",
         },
         incdirs={
            "lua_grdmap",
            "lib_hacks/code",
            "lua_grd",
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
   GIF_LIB = {
      header = "gif_lib.h",
      library = "libgif.a",
   },
   PNG_LIB = {
      header = "png.h",
      library = "libpng.a",
   },
   JPEG_LIB = {
      header = "jpeglib.h",
      library = "libjpeg.a",
   },
   ZZIP_LIB = {
      header = "zzip.h",
      library = "libzzip.a",
   },
}
dependencies={
 "lua >= 5.1 <= 5.2",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.grd/",
 license="MIT",
 summary="read/write/edit png/gif/jpg",
 detailed=[[

APNG is suported but you must have libpng configured for it ( which you 
probably do not )

 ]],
}

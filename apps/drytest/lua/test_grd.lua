local grd=require("grd")

g=assert(grd.create("GRD_FMT_U8_BGRA","dat/test.jpg","jpg"))

g:save("dat/out.png","png")


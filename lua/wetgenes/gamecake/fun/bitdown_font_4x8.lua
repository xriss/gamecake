
-- a legacy module

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.grd_mask=require("wetgenes.gamecake.fun.bitdown_font").build_grd(4,8)

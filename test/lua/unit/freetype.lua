
module(...,package.seeall)

local wstr=require("wetgenes.string")


function test_freetype()

local ft=require("wetgenes.freetype")
local grd=require("wetgenes.grd")

local fp=io.open("../mods/data/fonts/Vera.ttf","rb")
local d=fp:read("*a")
fp:close()

--local f=ft.create("../../../mods/data/fonts/Vera.ttf")
local f=ft.create()
f:load_data(d)

local g=grd.create()

f:size(64,64)
f:render(65)
f:grd(g)
g:convert(grd.FMT_U8_RGBA)

--print(wstr.dump(f))
--print(wstr.dump(g))

--g:save("test.png","png")

end



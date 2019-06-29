

module(...,package.seeall)

local wstr=require("wetgenes.string")

local grd=require("wetgenes.grd")

local grdcanvas=require("wetgenes.grdcanvas")


function test_fonts()

	local fonts=grdcanvas.canvas_fonts_create()

	for i,v in ipairs(fonts) do
		assert( fonts[i].g8:save("dat/grd/font_g8_"..i..".out.png","png") )
		assert( fonts[i].g32:save("dat/grd/font_g32_"..i..".out.png","png") )
	end

end


function test_text()

	local fonts=grdcanvas.canvas_fonts_create()

	for i,v in ipairs(fonts) do

		local g=grd.create("U8_INDEXED",12*v.hx,v.hy,1)
		g:palette(0,2,{0,0,0,0,255,255,255,255})
		local c=grdcanvas.canvas(g)
		c.set_font(i)
		c.text("Hello World!",0,0)

		assert( g:save("dat/grd/text_g8_"..i..".out.png","png") )
	end

end



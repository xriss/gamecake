

--
-- draw a font
--




module("fenestra.font")


--
-- Return a font object we can use to draw text with
--
-- "debug" is the built in 8x8 font
--
function setup(win,name)


	local font={}
	
	font.sx=8 -- base size of each char in this font
	font.sy=8
	
	font.px=0 -- current drawing position for this font
	font.py=0
	
	font.color=0xffffffff -- the current color to draw in
	
--
-- To simplify drawing the fonts are drawn not at the baseline but as their top/left corners
-- this way we can return a width and height of the total area neeeded to draw, you will need to handle
-- baseline offsets when positioning text. The whole baseline thing is a bit oldskool anyhow.
-- If you wish to have multiple sized fonts on a single line it is better if we make that
-- as hard as possible :)
--
-- also fonts are drawn in the directions of x++ and y--,
-- use open gl transforms to position the 0,0 wherever you want before drawing here
--	
	
--
-- cleanup this font
--
	function font.clean()
	end
	
--
-- set the position and color and size, any input may be nil for no change
--
	function font.set(px,py,color,sx,sy)
		font.px=px or font.px
		font.py=py or font.py
		font.color=color or font.color		
		font.sx=sx or font.sx
		font.sy=sy or sx or font.sy
		
	end
--
-- how big an area does this string require, return width,height
--
	function font.size(text,s)
		if s then font.sx=s font.sy=s end
		if text then return win.flat_measure({size=font.sx,s=text}) , font.sy end
	end
--
-- draw this string, optionally apply a different color to each char using the colors array
--
	function font.draw(text,colors)
		win.flat_print({x=font.px,y=font.py,size=font.sx,color=font.color,s=text,c=colors})
	end

	return font

end


--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,tilemap_text)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat


-- inject our text functions and vars into the base tilemap

tilemap_text.inject=function(it,opts)

-- if you load a 128x1 font data then the following functions can be used to print 7bit ascii text	

-- the current text window bounds, adjust for text wrapping / scrolling / etc
	it.text_px=0
	it.text_py=0
	it.text_hx=it.tilemap_hx
	it.text_hy=it.tilemap_hy

	it.text_fg=31 -- default foreground color index, white in swanky32
	it.text_bg=0  -- default background color index, transparent in swanky32
	
	it.text_tile4x8=function(c,fg,bg)
		return {(c:byte()),0,fg or it.text_fg,bg or it.text_bg} -- use the first line of tiles as a font
	end
	it.text_tile8x8=function(c,fg,bg)
		local i=(c:byte())
		local x=i%64
		local y=1+((i-x)/64) -- use tile lines 1 and 2
		return {x,y,fg or it.text_fg,bg or it.text_bg}
	end
	it.text_tile8x16=function(c,fg,bg)
		local i=(c:byte())
		local x=i%32
		local y=2+((i-x)/64) -- use tile lines 4,5,6,7 (or lines 2,3 in 8x16 tiles)
		return {x,y,fg or it.text_fg,bg or it.text_bg}
	end
	
-- replace the text_tile function if your font is somewhere else,
-- or you wish to handle more than 128 tiles (eg utf8)
-- here we pick one of the above functions based on the tile size

	if it.tile_hx==4 and it.tile_hy==8  then it.text_tile=it.text_tile4x8  end
	if it.tile_hx==8 and it.tile_hy==8  then it.text_tile=it.text_tile8x8  end
	if it.tile_hx==8 and it.tile_hy==16 then it.text_tile=it.text_tile8x16 end

	it.text_print_tile=function(c,x,y,fg,bg)
		it.tilemap_grd:pixels( x,y, 1,1, it.text_tile(c,fg,bg) )
	end

	it.text_print=function(s,x,y,fg,bg)
		for c in s:gmatch("([%z\1-\127\194-\244][\128-\191]*)") do
			if x>=it.text_px and y>=it.text_py and x<it.text_px+it.text_hx and y<it.text_py+it.text_hy then
				it.text_print_tile(c,x,y,fg,bg)
			end
			x=x+1
		end

		return x,y
	end

-- set the text window
	it.text_window=function(px,py,hx,hy)
		it.text_px=px or 0
		it.text_py=py or 0
		it.text_hx=hx or it.tilemap_grd.width
		it.text_hy=hy or it.tilemap_grd.height
	end

	it.text_window_center=function(hx,hy)
		it.text_px=math.floor((it.tilemap_grd.width-hx)/2)
		it.text_py=math.floor((it.tilemap_grd.height-hy)/2)
		it.text_hx=hx
		it.text_hy=hy
	end
	
	it.text_clear=function(c)
		it.tilemap_grd:clip(it.text_px,it.text_py,0,it.text_hx,it.text_hy,1):clear(c)
	end


	return it
end

	return tilemap_text
end



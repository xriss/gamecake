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
	
	it.screen_resize=function(hx,hy)
		if hx~=it.window_hx or hy~=it.window_hy then -- new size

			it.window_hx=hx
			it.window_hy=hy

			it.tilemap_hx=2+math.ceil(hx/it.tile_hx)
			it.tilemap_hy=2+math.ceil(hy/it.tile_hy)
			it.tilemap_grd:resize(it.tilemap_hx,it.tilemap_hy,1)

			it.text_hx=it.tilemap_hx
			it.text_hy=it.tilemap_hy

		end
	end


	it.text_tile4x8=function(c,fg,bg)
		return (c:byte()),0,fg or it.text_fg,bg or it.text_bg -- use the first line of tiles as a font
	end
	it.text_tile8x8=function(c,fg,bg)
		local i=(c:byte())
		local x=i%64
		local y=1+((i-x)/64) -- use tile lines 1 and 2
		return x,y,fg or it.text_fg,bg or it.text_bg
	end
	it.text_tile8x16=function(c,fg,bg)
		local i=(c:byte())
		local x=i%32
		local y=2+((i-x)/64) -- use tile lines 4,5,6,7 (or lines 2,3 in 8x16 tiles)
		return x,y,fg or it.text_fg,bg or it.text_bg
	end
	
-- replace the text_tile function if your font is somewhere else,
-- or you wish to handle more than 128 tiles (eg utf8)
-- here we pick one of the above functions based on the tile size

	if it.tile_hx==4 and it.tile_hy==8  then it.text_tile=it.text_tile4x8  end
	if it.tile_hx==8 and it.tile_hy==8  then it.text_tile=it.text_tile8x8  end
	if it.tile_hx==8 and it.tile_hy==16 then it.text_tile=it.text_tile8x16 end

	it.text_print_tile=function(c,x,y,fg,bg)
		it.tilemap_grd:pixels( x,y, 1,1, {it.text_tile(c,fg,bg)} )
	end

	it.text_print_image=function(image,x,y,fg,bg)
		fg=fg or 24
		bg=bg or 0
		local bm={}
		for py=0,image.hyt-1 do
			for px=0,image.hxt*2-1 do
				local bl=#bm
				bm[bl+1]=image.pxt*2+px
				bm[bl+2]=image.pyt+py
				bm[bl+3]=fg
				bm[bl+4]=bg
			end
		end
		it.tilemap_grd:pixels( x,y, image.hxt*2,image.hyt, bm )
	end

	it.text_print_border=function(image,x,y,w,h,gc,fg,bg)
		fg=fg or 31
		bg=bg or 0
		gc=gc or 1 -- size of centre repeat relative to edges
		local rx=math.floor(image.hxt*2/(gc+2)) -- tiles per edge segment
		local ry=math.floor(image.hyt/(gc+2))
		local sx=image.hxt*2-rx-rx -- tiles per center repeat
		local sy=image.hxt-ry-ry
		local x2=image.hxt*2-rx
		local y2=image.hyt-ry
		local bm={}
		for py=0,h-1 do
			for px=0,w-1 do

				local tx,ty

				if		py<ry		then	ty=(py)%ry					-- top line
				elseif	py>=h-ry	then	ty=(py-(h-ry-ry))%ry	+y2	-- bottom line
									else	ty=(py-ry)%sy			+ry	-- center lines
				end
				
				if		px<rx		then	tx=(px)%rx					-- left line
				elseif	px>=w-rx	then	tx=(px-(w-rx-rx))%rx	+x2	-- right line
									else	tx=(px-rx)%sx			+rx	-- center lines
				end

				local bl=#bm
				bm[bl+1]=image.pxt*2+tx
				bm[bl+2]=image.pyt+ty
				bm[bl+3]=fg
				bm[bl+4]=bg

			end
		end
		it.tilemap_grd:pixels( x,y, w,h, bm )
	end

	it.text_print1=function(s,x,y,fg,bg)
		local ox=x
		local bm={}
		for c in s:gmatch("([%z\1-\127\194-\244][\128-\191]*)") do
			if x>=it.text_px and y>=it.text_py and x<it.text_px+it.text_hx and y<it.text_py+it.text_hy then
				local c1,c2,c3,c4=it.text_tile(c,fg,bg)
				local bl=#bm
				bm[bl+1]=c1
				bm[bl+2]=c2
				bm[bl+3]=c3
				bm[bl+4]=c4
				x=x+1
			end
		end
		if x-ox>0 then
			it.tilemap_grd:pixels( ox,y, x-ox,1, bm )
		end
		return x,y
	end

	it.text_print2=function(s,x,y,fg,bg)
		local ox=x
		local bm={}
		for c in s:gmatch("([%z\1-\127\194-\244][\128-\191]*)") do
			if x>=it.text_px and y>=it.text_py and x<it.text_px+it.text_hx and y<it.text_py+it.text_hy then
				local c1,c2,c3,c4=it.text_tile(c,fg,bg)
				local bl=#bm
				bm[bl+1]=c1*2
				bm[bl+2]=c2
				bm[bl+3]=c3
				bm[bl+4]=c4
				bm[bl+5]=c1*2+1
				bm[bl+6]=c2
				bm[bl+7]=c3
				bm[bl+8]=c4
				x=x+2
			end
		end
		if x-ox>0 then
			it.tilemap_grd:pixels( ox,y, x-ox,1, bm )
		end
		return x,y
	end
	
	it.text_print=it.text_print1

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



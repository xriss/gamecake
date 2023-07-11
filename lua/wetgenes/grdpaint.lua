--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.grdpaint

	local wgrdpaint=require("wetgenes.grdpaint")

We use wgrdpaint as the local name of this library.

Add extra functionality to wetgenes.grd primarily these are functions that 
are used by swanky paint to manage its internal data.

Primarily we add a concept of "layers" and "history" these interfaces 
are added to a grd via a new object that lives inside the grd table and 
binds them together.

EG grd.history contains history data and functions.

As these are written first for swankypaint they may only work with 
indexed images and are currently in state of flux so may take a while 
to settle down.

]]


local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local grdpaint=M



-- slow but simple
local function countin3x3(g,cx,cy,t)
	cx=cx-1
	cy=cy-1
	local cw,ch=3,3
--clip
	if cx<0 then cw=cw+cx cx=0 end
	if cy<0 then ch=ch+cy cy=0 end
	if (cx+cw)>g.width  then cw=g.width -cx end
	if (cy+ch)>g.height then ch=g.height-cy end
	if cw<=0 or ch<=0 then return 0,0 end -- nothing

	local tab=g:pixels(cx,cy,cw,ch)
	local count=0
	for i,v in ipairs(tab) do
		if v==t then count=count+1 end
	end
	return count,#tab
end
-- slow but simple
local function countin3cross3(g,cx,cy,t)
	cx=cx-1
	cy=cy-1
	local cw,ch=3,3
--clip
	local x=cx+1
	local y=cy+1
	
	if cx<0 then cw=cw+cx cx=0 end
	if cy<0 then ch=ch+cy cy=0 end
	if (cx+cw)>g.width  then cw=g.width -cx end
	if (cy+ch)>g.height then ch=g.height-cy end
	if cw<=0 or ch<=0 then return 0,0 end -- nothing

	local tab={}
	if x>=0 and x<g.width then
		for i,v in ipairs(g:pixels(x,cy,1,ch)) do tab[#tab+1]=v end
	end
	if y>=0 and y<g.height then
		for i,v in ipairs(g:pixels(cx,y,cw,1)) do tab[#tab+1]=v end
	end
	local count=0
	for i,v in ipairs(tab) do
		if v==t then count=count+1 end
	end
	return count,#tab
end



-- perform a "smooth" rotation by using 3 shears.
-- Turns is a 0-1 value which maps to 0-360 deg or 0-2pi radians
-- yeah gonna need to replace that skew function with a proper shear
grdpaint.rots=function(gr,bg,turns)

	local rx= math.tan(math.pi*turns*2/2)
	local ry=-math.sin(math.pi*turns*2)

	local ga=grdpaint.skew(gr,bg,6,rx)
	local gb=grdpaint.skew(ga,bg,5,ry)
	local gc=grdpaint.skew(gb,bg,6,rx)

	return gc
end

grdpaint.skew=function(gr,bg,direction,ratio)

-- we need a new bigger grd depending on skew

	local g

	local slide_x
	local slide_y
	local flip_x
	local flip_y
	
	if     direction == 1 then slide_x=ratio flip_x=true flip_y=true
	elseif direction == 2 then slide_x=ratio flip_y=true
	elseif direction == 3 then slide_y=ratio flip_y=true
	elseif direction == 4 then slide_y=ratio
	elseif direction == 5 then slide_x=ratio flip_x=true
	elseif direction == 6 then slide_x=ratio
	elseif direction == 7 then slide_y=ratio flip_x=true flip_y=true
	elseif direction == 8 then slide_y=ratio flip_x=true
	end
	
	if slide_x then
		if slide_x<0 then
			local ir=(-slide_x)+1
			local ic=math.ceil(gr.height/ir)-1

			g=wgrd.create(wgrd.U8_INDEXED,gr.width+ic,gr.height,1)
			g:clear(bg)
			g:palette(0,256,gr) -- copy pal and pixels

			for i=0,ic do
				local px,py,hx,hy = i , i*ir , gr.width , ir
				if flip_x then px=g.width-gr.width-px end
				if flip_y then py=g.height-(py+ir) end
				g:pixels(px,py,hx,hy,gr:pixels(0,py,hx,hy,""))
			end
		else
			local ir=slide_x+1
			local ic=math.ceil(gr.height*ir)-1

			g=wgrd.create(wgrd.U8_INDEXED,gr.width+(gr.height-1)*ir,gr.height,1)
			g:clear(bg)
			g:palette(0,256,gr) -- copy pal and pixels

			for i=0,gr.height-1 do
				local px,py,hx,hy = i*ir , i , gr.width , 1
				if flip_x then px=g.width-gr.width-px end
				if flip_y then py=g.height-(py+1) end
				g:pixels(px,py,hx,hy,gr:pixels(0,py,hx,hy,""))
			end
		end
	end
	if slide_y then
		if slide_y<0 then
			local ir=(-slide_y)+1
			local ic=math.ceil(gr.width/ir)-1

			g=wgrd.create(wgrd.U8_INDEXED,gr.width,gr.height+ic,1)
			g:clear(bg)
			g:palette(0,256,gr) -- copy pal and pixels

			for i=0,ic do
				local px,py,hx,hy = i*ir , i , ir , gr.height
				if flip_x then px=g.width-(px+ir) end
				if flip_y then py=g.height-gr.height-py end
				g:pixels(px,py,hx,hy,gr:pixels(px,0,hx,hy,""))
			end
		else
			local ir=slide_y+1
			local ic=math.ceil(gr.width*ir)-1

			g=wgrd.create(wgrd.U8_INDEXED,gr.width,gr.height+(gr.width-1)*ir,1)
			g:clear(bg)
			g:palette(0,256,gr) -- copy pal and pixels

			for i=0,gr.width-1 do
				local px,py,hx,hy = i , i*ir , 1 , gr.height
				if flip_x then px=g.width-(px+1) end
				if flip_y then py=g.height-gr.height-py end
				g:pixels(px,py,hx,hy,gr:pixels(px,0,hx,hy,""))
			end
		end
	end



	return g
end


grdpaint.outline=function(gr,bg,fg)

-- we need a new grd 2 pixels bigger

	local g=wgrd.create(wgrd.U8_INDEXED,gr.width+2,gr.height+2,1)

	g:clear(bg.i)
	g:palette(0,256,gr) -- copy pal and pixels
	g:pixels(1,1,gr.width,gr.height,gr:pixels(0,0,gr.width,gr.height,""))

	local tab=g:pixels(0,0,g.width,g.height)

		for x=0,g.width-1 do
			for y=0,g.height-1 do
				local t=tab[1+x+y*g.width]
				if t==bg.i then
					local found,count=countin3cross3(gr,x-1,y-1,bg.i)
					if found~=count and count>0 then -- any non background pixels
						tab[1+x+y*g.width]=fg.i
					end
				end
			end
		end

	g:pixels(0,0,g.width,g.height,tab)

	return g
end

grdpaint.square_outline=function(gr,bg,fg)

-- we need a new grd 2 pixels bigger

	local g=wgrd.create(wgrd.U8_INDEXED,gr.width+2,gr.height+2,1)

	g:clear(bg.i)
	g:palette(0,256,gr) -- copy pal and pixels
	g:pixels(1,1,gr.width,gr.height,gr:pixels(0,0,gr.width,gr.height,""))

	local tab=g:pixels(0,0,g.width,g.height)

		for x=0,g.width-1 do
			for y=0,g.height-1 do
				local t=tab[1+x+y*g.width]
				if t==bg.i then
					local found,count=countin3x3(gr,x-1,y-1,bg.i)
					if found~=count and count>0 then -- any non background pixels
						tab[1+x+y*g.width]=fg.i
					end
				end
			end
		end

	g:pixels(0,0,g.width,g.height,tab)

	return g
end



grdpaint.inline=function(gr,bg,fg)

-- we need a new grd 2 pixels smaller

	local g=wgrd.create(wgrd.U8_INDEXED,gr.width-2,gr.height-2,1)

	g:clear(bg.i)
	g:palette(0,256,gr) -- copy pal and pixels
	
	if gr.width<=2 or gr.height<=2 then return g end -- empty result
	
	g:pixels(0,0,gr.width-2,gr.height-2,gr:pixels(1,1,gr.width-2,gr.height-2,""))

	local tab=g:pixels(0,0,g.width,g.height)

		for x=0,g.width-1 do
			for y=0,g.height-1 do
				local t=tab[1+x+y*g.width]
				if t~=bg.i then
					local found,count=countin3cross3(gr,x+1,y+1,bg.i)
					if found>0 then -- any background pixels
						tab[1+x+y*g.width]=bg.i
					end
				end
			end
		end

	g:pixels(0,0,g.width,g.height,tab)

	return g
end




grdpaint.rotate=function(gr,d)

	local g=wgrd.create(wgrd.U8_INDEXED,gr.height,gr.width,1)

	g:palette(0,256,gr) -- copy pal

	local tab=gr:pixels(0,0,gr.width,gr.height)
	local rot={}

	if d and d<0 then
		for x=0,g.width-1 do
			for y=0,g.height-1 do
				rot[1+x+y*g.width]=tab[1+(gr.width-y-1)+x*gr.width]
			end
		end
	else
		for x=0,g.width-1 do
			for y=0,g.height-1 do
				rot[1+x+y*g.width]=tab[1+y+(gr.height-x-1)*gr.width]
			end
		end
	end

	g:pixels(0,0,g.width,g.height,rot)

	return g
end

-- map a 32bit image down to 8bit
grdpaint.map8=function(g,p,colors,dither)

	local r=wgrd.create(wgrd.U8_INDEXED,g.width,g.height,g.depth)

	r:palette(0,256,p) -- copy pal
	
	g:remap(r,colors,dither)

	return r
end

-- find all pixels connected to the color at x,y and return a grd mask
grdpaint.fill_mask=function(g,x,y)
	if x<0 or y<0 or x>=g.width or y>=g.height then return end -- out of range
	local r=wgrd.create(wgrd.U8_INDEXED,g.width,g.height,1)

	assert( g:fillmask(r,x,y) )

--[[
	local idx=g:pixels(x,y,1,1)
	idx=idx[1]
	
	r:pixels(x,y,1,1,{1}) -- first fill
	local ps={[y*0x10000+x]=true} -- array of active pixels we are filling

	local leaks={{-1,0},{1,0},{0,-1},{0,1}} -- how fill spreads

	local done=false
	while not done do done=true
		for p,_ in pairs(ps) do local x,y=p%0x10000,math.floor(p/0x10000)
			for _,v in ipairs(leaks) do
				local px,py=x+v[1],y+v[2]
				if px>=0 and py>=0 and px<g.width and py<g.height then
					local pyx=py*0x10000+px
					local t=g:pixels(px,py,1,1)
					if #t and t[1]==idx then -- a pixel to fill
						local t=r:pixels(px,py,1,1)
						if t[1]==0 then -- need to fill
							r:pixels(px,py,1,1,{1}) -- fill
							ps[pyx]=true -- mark as active
							done=false
						end
					end
				end
			end
			
			ps[p]=nil -- filled
		end
	end
]]
	
	return r
end

-- a paint that wraps at the edges
grdpaint.paintwrap=function(ga,gb,x,y,cx,cy,cw,ch,mode,trans,color)

	local w=ga.width
	local h=ga.height

	x=((x%w)+w)%w
	y=((y%h)+h)%h

	if not cx then -- full gb
		cx=0
		cy=0
		cw=gb.width
		ch=gb.height
	end
	
	if x+cw > w then
		if y+ch > h then
			ga:paint(gb,x-w,y-h,cx,cy,cw,ch,mode,trans,color)
		end
		ga:paint(gb,x-w,y,cx,cy,cw,ch,mode,trans,color)
	end
	if y+ch > h then
		ga:paint(gb,x,y-h,cx,cy,cw,ch,mode,trans,color)
	end
	ga:paint(gb,x,y,cx,cy,cw,ch,mode,trans,color)

end

-- above are old swankypaint functions that should be moved into the canvas functions below.
-- and probably shifted into C for speed as they are a tad slow...


-- moved canvas , history and layers into seperate files which means this file will eventually become obsolete

grdpaint.canvas  = require("wetgenes.grdcanvas").canvas
grdpaint.history = require("wetgenes.grdhistory").history
grdpaint.layers  = require("wetgenes.grdlayers").layers



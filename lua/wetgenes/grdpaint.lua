--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local cmsgpack=require("cmsgpack")

local zlib=require("zlib")
local inflate=function(d) return ((zlib.inflate())(d)) end
local deflate=function(d) return ((zlib.deflate())(d,"finish")) end

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






grdpaint.outline=function(gr,bg,fg)

-- we need a new grd 2 pixels bigger

	local g=wgrd.create(wgrd.U8_INDEXED,gr.width+2,gr.height+2,1)

	g:clear(bg.i)
	g:palette(0,256,gr:palette(0,256)) -- copy pal and pixels
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




grdpaint.inline=function(gr,bg,fg)

-- we need a new grd 2 pixels smaller

	local g=wgrd.create(wgrd.U8_INDEXED,gr.width-2,gr.height-2,1)

	g:clear(bg.i)
	g:palette(0,256,gr:palette(0,256)) -- copy pal and pixels
	
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

	g:palette(0,256,gr:palette(0,256)) -- copy pal

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

	r:palette(0,256,p:palette(0,256)) -- copy pal
	
	g:remap(r,colors,dither)

	return r
end

-- find all pixels connected to the color at x,y and return a grd mask
grdpaint.fill_mask=function(g,x,y)
	if x<0 or y<0 or x>=g.width or y>=g.height then return end -- out of range
	local r=wgrd.create(wgrd.U8_INDEXED,g.width,g.height,1)
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
-- and probably shifted into C for speed...

-- create a canvas state within the given grd
grdpaint.canvas=function(grd)

	local canvas={grd=grd}
	grd.canvas=canvas
	
	canvas.clip=function(x,y,z,w,h,d)
		if type(x)~="number" then -- clear
			canvas.grd_clipped=nil
		elseif type(y)~="number" then -- frame pick
			canvas.grd_clipped=canvas.grd:clip(0,0,x,canvas.grd.width,canvas.grd.height,1)
		else
			if not h then -- 2d
				h=w w=z z=0 d=1
			end
			canvas.grd_clipped=canvas.grd:clip(px,py,pz,hx,hy,hz)
		end
	end

	canvas.fill=function(mode)
		canvas.fill_mode = mode or canvas.fill_mode or "edge" -- "fill" ?
	end
	canvas.fill()

	canvas.color=function(fgc,bgc,mode)
		canvas.color_foreground = fgc or canvas.color_foreground or 1
		canvas.color_background = bgc or canvas.color_background or 0
		canvas.color_mode       = mode or canvas.color_mode or "color"
	end
	canvas.color()
	
	canvas.brush_handle=function(oxf,oyf)
		canvas.brush_oxf = oxf or canvas.brush_oxf or 0.5
		canvas.brush_oyf = oyf or canvas.brush_oyf or 0.5

		canvas.brush_ox = math.floor(canvas.brush_hx*canvas.brush_oxf)
		canvas.brush_oy = math.floor(canvas.brush_hy*canvas.brush_oyf)
	end
	canvas.brush=function(hx,hy,mode)
		canvas.brush_hx   = math.floor( (hx or canvas.brush_hx or 1) + 0.5 )
		canvas.brush_hy   = math.floor( (hy or canvas.brush_hy or 1) + 0.5 )
		canvas.brush_mode = mode or canvas.brush_mode or "box"

		canvas.brush_handle()
	end
	canvas.brush()

	canvas.clear=function(c)
		local g=canvas.grd_clipped or canvas.grd
		g:clear(c or canvas.color_background)
	end
	
	canvas.plot=function(x,y)
		local px,py,hx,hy=math.floor(0.5+x-canvas.brush_ox),math.floor(0.5+y-canvas.brush_oy),canvas.brush_hx,canvas.brush_hy
		local g=canvas.grd_clipped or canvas.grd
		if px<0 then hx=hx+px px=0 end
		if py<0 then hy=hy+py py=0 end
		if px+hx>g.width  then hx=g.width -px end
		if py+hy>g.height then hy=g.height-py end
		if hx>0 and hy>0 then
			g:clip(px,py,0,hx,hy,1):clear(canvas.color_foreground)
		end
	end

	canvas.line=function(x1,y1,x2,y2)
		if x1 > x2 then x1,x2=x2,x1 end
		if y1 > y2 then y1,y2=y2,y1 end
		
		local dx=math.floor(0.5+x2-x1)
		local dy=math.floor(0.5+y2-y1)
		
		if dx>dy then
			for i=0,dx,1 do
				local d=math.floor(0.5 + (dy*i/dx) )
				canvas.plot(x1+i,y1+d)
			end
		else
			for i=0,dy,1 do
				local d=math.floor(0.5 + (dx*i/dy) )
				canvas.plot(x1+d,y1+i)
			end
		end
				
	end

	canvas.box=function(x1,y1,x2,y2)
		if x1 > x2 then x1,x2=x2,x1 end
		if y1 > y2 then y1,y2=y2,y1 end

		canvas.line(x1,y1,x2,y1)
		canvas.line(x2,y1,x2,y2)
		canvas.line(x2,y2,x1,y2)
		canvas.line(x1,y2,x1,y1)
	end

	canvas.circle=function(x,y,r)
	
		if r<0 then r=-r end
	
		local r2=math.ceil(math.sqrt(r*r/2))

		for i=0,r2,1 do
			local d=math.ceil(math.sqrt( r*r - i*i ))

			canvas.plot(x+i,y-d)
			canvas.plot(x-i,y+d)
			canvas.plot(x-d,y+i)
			canvas.plot(x+d,y-i)
			canvas.plot(x-i,y-d)
			canvas.plot(x+i,y+d)
			canvas.plot(x-d,y-i)
			canvas.plot(x+d,y+i)
		end

	end

	return canvas

end


local palette_nil=("\0\0\0\0"):rep(256) -- an empty all 0 palette

-- create a history state within the given grd
grdpaint.history=function(grd)

	local history={grd=grd}	
	grd.history=history
	
	if not grd.layers then grdpaint.layers(grd) end -- need layers

	history.config=function()
		history.length=0
		history.list={}
		history.frame=0
		history.index=0
	end
	history.config()
	
	history.get=function(index)
		if not index then return end
		local it=history.list[index]
		it=it and cmsgpack.unpack(inflate(it))
		return it
	end
	history.set=function(index,it)
		it=it and deflate(cmsgpack.pack(it))
		history.list[index]=it
		if index>history.length then history.length=index end
	end
	
-- take a snapshot of this frame for latter diffing (started drawing on this frame only)
	history.draw_begin=function(x,y,z,w,h,d)
		history.area={
			x or 0 ,
			y or 0 ,
			z or 0 ,
			w or history.grd.width -(x or 0) ,
			h or history.grd.height-(y or 0) ,
			d or 1 }
		if w==0 and h==0 and d==0 then -- flag palette only with auto merge
			history.area.pal=true
			if not history.pal then -- use original
				history.grd_diff=history.grd:clip(unpack(history.area)):duplicate()
			end
		else
			history.pal=nil -- stop accumulating palette changes
			history.grd_diff=history.grd:clip(unpack(history.area)):duplicate()
		end
	end

-- return a temporray grd of only the frame we can draw into
	history.draw_get=function()
		assert(history.grd_diff) -- sanity
		return history.grd:clip(unpack(history.area))
	end

-- revert back to begin state
	history.draw_revert=function()
		assert(history.grd_diff) -- sanity
		local c=history.area
		history.grd:pixels(c[1],c[2],c[3],c[4],c[5],c[6],history.grd_diff) -- restore image
		history.grd:palette(0,256, history.grd_diff:palette(0,256,"") ) -- restore palette
	end
	
-- stop any accumulated changes
	history.draw_end=function()
		history.grd_diff=nil
		history.pal=nil
	end
	
-- push any changes we find into the history
	history.draw_save=function()
		assert(history.grd_diff) -- sanity
		
		if history.area.pal then -- palette only with auto merge
		
			if history.pal then -- auto merge
			
				local ga=history.grd_diff:duplicate()
				local it=history.pal
				local gb=history.grd:clip(unpack(history.area))
				ga:xor(gb)
				it.palette=ga:palette(0,256,"")
				history.set(it.id,it)

			else -- new
				local ga=history.grd_diff:duplicate()
				local it={}
				local gb=history.grd:clip(unpack(history.area))
				ga:xor(gb)
				it.palette=ga:palette(0,256,"")
				if it.palette~=palette_nil then -- only remember if any colour has changed

					it.prev=history.index -- link to prev
					it.id=history.length+1

					history.index=it.id
					history.set(it.id,it)
					if it.prev then -- link from prev
						local pit=history.get(it.prev)
						if pit then
							pit.next=it.id -- link to *most*recent* next
							history.set(pit.id,pit)
						end
					end

					history.pal=it
				end
			end

		else
		
			local ga=history.grd_diff
			local it={x=0,y=0,z=0,w=ga.width,h=ga.height,d=ga.depth}
			local gb=history.grd:clip(unpack(history.area))
			ga:xor(gb)
			it.palette=ga:palette(0,256,"")
			if it.palette==palette_nil then it.palette=nil end -- no colour has changed so do not store diff
			ga:shrink(it)
			if it.w>0 and it.h>0 and it.d>0 then -- some pixels have changed
				it.data=ga:pixels(it.x,it.y,it.z,it.w,it.h,it.d,"") -- get minimal xored data area as a string
			else
				it.x=nil
				it.y=nil
				it.z=nil
				it.w=nil
				it.h=nil
				it.d=nil
			end

			it.prev=history.index>0 and  history.index -- link to prev
			it.id=history.length+1

			history.index=it.id
			history.set(it.id,it)
			if it.prev then -- link from prev which must exist
				local pit=history.get(it.prev)
				pit.next=it.id -- link to *most*recent* next
				history.set(pit.id,pit)
			end
			
			history.draw_end()
		end
	end
	
	history.apply=function(index) -- apply diff at this index
		history.draw_end()
		local it=history.get(index or history.index) -- default to current index
		if it and it.w and it.h and it.d and it.data then -- xor image
			local ga=wgrd.create(history.grd.format,it.w,it.h,it.d)
			local gb=history.grd:clip(it.x,it.y,it.z,it.w,it.h,it.d)
			ga:pixels(0,0,0,it.w,it.h,it.d,it.data)
			if it.palette then -- xor pal
				ga:palette(0,256,it.palette)
			end
			gb:xor(ga)
		elseif it.palette then -- xor pal only
			local ga=wgrd.create(history.grd.format,0,0,0)
			ga:palette(0,256,it.palette)
			history.grd:clip(0,0,0,0,0,0):xor(ga)
		end
	end

	history.goto=function(index) -- goto this undo index
	
		-- this will work if we are on the right branch
		-- and destination is in the future
		while index>history.index do 
			if not history.redo() then break end
		end
		
		-- this will work if we are on the right branch
		-- and destination is in the past
		while index<history.index do
			if not history.undo() then break end
		end
		
		-- did not work so we need to find a common point in history
		if index~=history.index then -- need to find shared ancestor		
			-- TODO make this work, right now branches can get lost...
		end

	end

	history.undo=function() -- go back a step
		local it=history.get(history.index)
		if it and it.prev and history.list[it.prev] then -- somewhere to go
			history.apply(history.index)
			history.index=it.prev
			return true
		end
	end

	history.redo=function(id) -- go forward a step
		if not id then
			local it=history.get(history.index)
			id=it and it.next
		end
		if id then -- somewhere to go
			history.apply(id)
			history.index=id
			return true
		end
	end
	
	local saveme={
		"length",
		"list",
		"frame",
		"index",
	}

	history.save=function()

		history.draw_end()

		local it={}
		
		for _,n in ipairs(saveme) do
			it[n]=history[n]
		end

		return deflate(cmsgpack.pack(it))
	end
	
	history.load=function(it)

		history.draw_end()
		
		local it=cmsgpack.unpack(inflate(it))

		for _,n in ipairs(saveme) do
			history[n]=it[n]
		end
	end

-- return amount of memory used by undo
	history.get_memory=function()
		local total=0
		local count=0
		local mini=0
		for i=history.length,0,-1 do
			local v=history.list[i]
			if v then
				total=total+#v
				count=count+1
				mini=i
			end
		end
		return total,count,mini
	end

	return history
end



-- create a layers state within the given grd
-- layers are just a way of breaking one grd into discreet 2d areas
-- which can then be, optionally, recombined into a final image/anim
grdpaint.layers=function(grd)

	local layers={grd=grd}	
	grd.layers=layers

	layers.frame=0 -- keep track of frame

	-- set layer config, default to entire frame.
	layers.config=function(x,y,n)
		if n then
			if not x and not y then
				x=math.floor(math.sqrt(n))
			end
			if x and not y then
				y=math.ceil(n/x)
			end
			if y and not x then
				x=math.ceil(n/y)
			end
		end
		x=x or 1
		y=y or 1
		layers.x=x -- number of layers wide
		layers.y=y -- number of layers high
		layers.count=n or x*y -- total layers is optional
		layers.idx=0
	end
	layers.config() -- defaults

	-- get the size of each layer
	layers.size=function()
		return math.floor(layers.grd.width/layers.x),math.floor(layers.grd.height/layers.y)
	end

	-- return a 3d clip area to get a single layer from the grd
	layers.area=function(idx,frame)
		idx=idx or layers.idx
		frame=frame or layers.frame
		if idx==0 then -- layer 0 is special case, full size
			return 0,0,frame, layers.grd.width,layers.grd.height,1
		end
		local lw,lh=layers.size()
		local lx=lw*math.floor((idx-1)%layers.x)
		local ly=lh*math.floor((idx-1)/layers.x)
		return  lx,ly,frame, lw,lh,1
	end

	-- get a new grd of all layers merged for every frame
	layers.flatten_grd=function()
		local gw,gh,gd=layers.grd.width,layers.grd.height,layers.grd.depth
		local lw,lh=layers.size()
		local g=wgrd.create(wgrd.U8_INDEXED,iw,ih,gd) -- new size same depth
		g:palette(0,256,grd:palette(0,256))
		for z=0,gd-1 do
			for i=layers.count-1,0,-1 do
				local x=math.floor(i%layers.x)
				local y=math.floor(i/layers.x)
				g:clip(0,0,z,iw,ih,1):paint( grd:clip(0,0,z,gw,gh,1) ,0,0,x*iw,y*ih,iw,ih,wgrd.PAINT_MODE_ALPHA,-1,-1)
			end
		end
		return g
	end

	return layers

end



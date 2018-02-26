--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

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
grdpaint.map8=function(g,p)

	local ps=p:palette(0,256)

	local r=wgrd.create(wgrd.U8_INDEXED,g.width,g.height,g.depth)
	r:palette(0,256,ps) -- copy pal
	
	local function best(p)
		local b=0
		local d=256*256*4*4
		for i=0,255 do
			local d1=ps[1+i*4]-p[1]
			local d2=ps[2+i*4]-p[2]
			local d3=ps[3+i*4]-p[3]
			local d4=ps[4+i*4]-p[4]
			local dd=d1*d1+d2*d2+d3*d3+d4*d4*2
			if dd<=d then
				d=dd
				b=i
			end
		end
		return b
	end

	for z=0,g.depth-1 do
		for y=0,g.height-1 do
			for x=0,g.width-1 do
				local p=g:pixels(x,y,z,1,1,1)
				r:pixels(x,y,z,1,1,1,{best(p)})
			end
		end
	end
	
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



-- create a canvas state with the given opts
grdpaint.canvas=function(grd)

	local canvas={}

	canvas.grd=grd -- what we draw into
	
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




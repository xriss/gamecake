--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.grdcanvas

	local wgrdcanvas=require("wetgenes.grdcanvas")

We use wgrdcanvas as the local name of this library.


]]


local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local bit=require("bit")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local grdcanvas=M


-- createa a grd of ramps of the given width and #ramps height
grdcanvas.ramps=function(w,rs)
	local h=#rs
	local g=wgrd.create(wgrd.FMT_U8_RGBA_PREMULT,w,h,1)
	g:clear(0xffffffff)

	for idx,v in ipairs(rs) do
		grdcanvas.cmap_ramp( g:clip( 0,idx-1,0, w,1,1 ) , v )
	end

	return g
end

-- cmap_ramp but create keys from a very simple ramp
grdcanvas.cmap_ramp_simple=function(g,cr,cg,cb,ca)

	if not cg then -- rgba in 0xaarrggbb format
		cr,cg,cb,ca=wpack.argb8_b4(cr) -- unpack it from ca only
	end
	
-- ca is *not* alpha it is +- range of ramp

	if cr<0   then cr=0   end
	if cr>255 then cr=255 end
	if cg<0   then cg=0   end
	if cg>255 then cg=255 end
	if cb<0   then cb=0   end
	if cb>255 then cb=255 end
	if ca<0   then ca=0   end
	if ca>255 then ca=255 end

	local crl=cr-ca
	local cgl=cg-ca
	local cbl=cb-ca
	
	if crl<0   then crl=0   end
	if crl>255 then crl=255 end
	if cgl<0   then cgl=0   end
	if cgl>255 then cgl=255 end
	if cbl<0   then cbl=0   end
	if cbl>255 then cbl=255 end
	
	local crh=cr+ca
	local cgh=cg+ca
	local cbh=cb+ca
	
	if crh<0   then crh=0   end
	if crh>255 then crh=255 end
	if cgh<0   then cgh=0   end
	if cgh>255 then cgh=255 end
	if cbh<0   then cbh=0   end
	if cbh>255 then cbh=255 end
	
	local keys={
		{ argb=wpack.b4_argb8(crl,cgl,cbl,0xff), value=0.0 },
		{ argb=wpack.b4_argb8(cr ,cg ,cb ,0xff), value=0.5 },
		{ argb=wpack.b4_argb8(crh,cgh,cbh,0xff), value=1.0 },
	}
	
	return grdcanvas.cmap_ramp(g,keys)
end

-- create a blended ramp of colors left to right in this single pixel high bitmap ( a cmap )
grdcanvas.cmap_ramp=function(g,keys,opts)

	if type(keys)=="number" then
		return grdcanvas.cmap_ramp_simple(g,keys)
	end
	
	opts=opts or {}

	local idx=1
	local lastkey=keys[idx]
	local nextkey=keys[idx]
	
	local cs={}
	local ci=0

	local blend=function(x,f)
	
		if f<=0 then f=0 end
		if f>=1 then f=1 end
		local l=1-f
		
		local clamp=function(a) if a<0 then a=0 elseif a>255 then a=255 end return math.floor(a) end

		local a1=bit.band(bit.rshift(lastkey.argb,24),0xff)
		local r1=bit.band(bit.rshift(lastkey.argb,16),0xff)
		local g1=bit.band(bit.rshift(lastkey.argb, 8),0xff)
		local b1=bit.band(lastkey.argb,0xff)

		local a2=bit.band(bit.rshift(nextkey.argb,24),0xff)
		local r2=bit.band(bit.rshift(nextkey.argb,16),0xff)
		local g2=bit.band(bit.rshift(nextkey.argb, 8),0xff)
		local b2=bit.band(nextkey.argb,0xff)

		local a=a1*l + a2*f
		local r=r1*l + r2*f
		local g=g1*l + g2*f
		local b=b1*l + b2*f
		
		cs[x*4+1]=clamp(r)
		cs[x*4+2]=clamp(g)
		cs[x*4+3]=clamp(b)
		cs[x*4+4]=clamp(a)

	end

	local f=0
	local m=(g.width-1) ; if m==0 then m=1 end
	for x=0,(g.width-1) do
		f=x/m	-- fractional location
		if f>=nextkey.value then
			idx=idx+1
			lastkey=nextkey
			nextkey=keys[idx]
			if not nextkey then nextkey=lastkey end -- run out of keys
		end
		
		if f<=lastkey.value then -- before
			blend(x,0)
		elseif f>=nextkey.value then --after
			blend(x,1)
		else -- middle
			local d=nextkey.value-lastkey.value
			if d<=0 then d=1 end
			blend(x, (f-lastkey.value)/d )
		end
		
	end
	dprint(cs)
	g:pixels(0,0,0,g.width,1,1,cs)
	
	return g
end


grdcanvas.canvas_fonts_create=function()
	if grdcanvas.canvas_fonts then return grdcanvas.canvas_fonts end -- already done
	grdcanvas.canvas_fonts={}
	local fonts=grdcanvas.canvas_fonts

	local bitdown=require("wetgenes.gamecake.fun.bitdown") -- reuse bitdown code to process strings
	local funfont64=require("wetgenes.gamecake.fun.funfont64") -- raw data
	
	local cmap=bitdown.cmap_build({
		name="white",
		[ 0]={bgra=0x00000000,code=". ",name="transparent"},
		[ 1]={bgra=0xffffffff,code="7 ",name="white"},
	})


	local render=function(hx,hy,style)
		style=style or ""
		local font={}
	
		local data=assert(funfont64["data"..hx.."x"..hy..style]) -- get data and check size is valid

		font.idx=#fonts+1
		font.name="fun"..hx.."x"..hy..style
		font.hx=hx
		font.hy=hy

		font.g8=wgrd.create("U8_INDEXED",128*hx,2*hy,1) -- 8bit version
		font.g8:palette(0,2,{0,0,0,0,255,255,255,255})

		font.g32=wgrd.create("U8_RGBA_PREMULT",128*hx,2*hy,1) -- 32bit version

		for i=0,255 do
			local s=data[i]
			if s then
				bitdown.pix_grd(s,cmap,font.g32,(i%128)*hx,math.floor(i/128)*hy,hx,hy)
			end
		end
		font.g32:remap(font.g8,2,0) -- convert to 2 color indexed version for painting with
		
		fonts[font.idx]=font
		fonts[font.name]=font
	end
	
	render(4,8)
	render(8,8)
	render(8,16,"")
	render(8,16,"r")
	render(8,16,"i")
	
	
	return grdcanvas.canvas_fonts
end


-- create a canvas state within the given grd
grdcanvas.canvas=function(grd)

	local canvas={grd=grd}
	grd.canvas=canvas
	
-- get available base fonts creating if needed
	canvas.fonts=function()
		if grdcanvas.canvas_fonts then return grdcanvas.canvas_fonts end -- already done
		return grdcanvas.canvas_fonts_create()
	end
	
-- set default font
	canvas.set_font=function(font)
		if type(font)~="table" then font=(canvas.fonts())[font] end -- auto lookup by name
		canvas.font=font
		return font
	end
	
-- render some text into this canvas (8 bit only)
	canvas.text=function(s,x,y,font)
		font=font or canvas.font -- default font
		if not font then font=canvas.set_font("fun4x8") end -- use 4x8 if font is missing
		
		for c in s:gmatch("([%z\1-\127\194-\244][\128-\191]*)") do
			canvas.grd:paint(font.g8,x,y,(c:byte()%128)*font.hx,0,font.hx,font.hy,grd.PAINT_MODE_COLOR,0,canvas.color_foreground)
			x=x+font.hx
		end

	end

	
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



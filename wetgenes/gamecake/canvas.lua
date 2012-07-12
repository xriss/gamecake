-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require





module("wetgenes.gamecake.canvas")

base=require(...)
meta={}
meta.__index=base

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")

--local nacl=require("wetgenes.nacl.core")

function bake(opts)

	local canvas={}
	setmetatable(canvas,meta)
	
	canvas.cake=opts.cake
	canvas.gl=opts.gl
	
	canvas.width=opts.width or 320
	canvas.height=opts.height or 240
	
	canvas.fmt=opts.cake.canvas_fmt

	canvas:start()
	
	return canvas
end

grd_blit = function(canvas,t,cx,cy,ix,iy,w,h)
	assert(
			canvas.grd:blit(	t,
								cx,cy,
								ix,iy,
								w,h)
	)
end

gl_blit = function(canvas,t,cx,cy,ix,iy,w,h,cw,ch)

--print("gl_blit + ",nacl.time()," ",t.filename )

	local gl=canvas.gl
	
--	cx=(cx or 0)+t.x -- handle adjustment of the x,y position
--	cy=(cy or 0)+t.y

	ix=ix or 0
	iy=iy or 0
	w=w or t.width
	h=h or t.height
	cw=cw or w
	ch=ch or h
	
	if cw==0 or ch==0 then return end -- nothing to draw
	
	local tw=t.texture_width -- hacks for cards that do not suport non power of two texture sizes
	local th=t.texture_height -- we just use a part of this bigger texture
	
	local cxw=cx+cw
	local cyh=cy+ch
	
	local ixw=(ix+w)/tw
	local iyh=(iy+h)/th
	
	ix=ix/tw
	iy=iy/th

	pack.save_array({
		cx,		cy,		0,		ix,		iy,
		cxw,	cy,		0,		ixw,	iy,
		cx,		cyh,	0,		ix,		iyh,
		cxw,	cyh,	0,		ixw,	iyh,
	},"f32",0,5*4,canvas.vdat)	

--	gl.EnableClientState(gl.VERTEX_ARRAY)
--	gl.EnableClientState(gl.TEXTURE_COORD_ARRAY)
	
--print("gl_blit . ",nacl.time() )

	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.vbuf)
	gl.BufferData(gl.ARRAY_BUFFER,5*4*4,canvas.vdat,gl.DYNAMIC_DRAW)

	gl.VertexPointer(3,gl.FLOAT,5*4,0*0)
	gl.TexCoordPointer(2,gl.FLOAT,5*4,3*4)

	gl.BindTexture(gl.TEXTURE_2D,t.id)

	gl.DrawArrays(gl.TRIANGLE_STRIP,0,4)

--print("gl_blit - ",nacl.time() )

end




start = function(canvas)
	if canvas.gl then -- open
		canvas.vbuf=canvas.gl.GenBuffer()
		canvas.vdat=pack.alloc(4*5*4) -- temp vertex quad draw buffer		
		blit=gl_blit
	else
		canvas.grd=assert(wgrd.create(canvas.fmt,
		canvas.width, canvas.height, 1))
		blit=grd_blit
	end
end

stop = function(canvas)
	if canvas.gl then -- free
		if canvas.vbuf then canvas.gl.DeleteBuffer(canvas.vbuf) canvas.vbuf=nil end
		canvas.vdat=nil
		blit=nil
	else
		canvas.grd=nil
		blit=nil
	end
end




font_set = function(canvas,font)
	canvas.font=font or canvas.font
	canvas:font_set_size(16,0)
	canvas:font_set_xy(0,0)
end

font_set_size = function(canvas,size,add)
	canvas.font_size=size
	canvas.font_add=add or 0 -- clear the x space tweak
end
font_set_xy = function(canvas,x,y)
	canvas.font_x=x or canvas.font_x
	canvas.font_y=y or canvas.font_y
end

font_width=function(canvas,text)

	local font=canvas.font
	local s=canvas.font_size/font.size
	local x=0
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font.chars[cid] or canvas.font.chars[32]
		
		x=x+(c.add*s)+canvas.font_add
	end

	return x
end

font_draw=function(canvas,text)

	local x=canvas.font_x
	local y=canvas.font_y
	local font=canvas.font
	
	local s=canvas.font_size/font.size
	
	local gl=canvas.gl

	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font.chars[cid] or canvas.font.chars[32]

		canvas:blit(c,x+(c.x*s),y+(c.y*s),nil,nil,c.width,c.height,c.width*s,c.height*s)

		x=x+(c.add*s)+canvas.font_add
	end
	
	canvas.font_x=x
end


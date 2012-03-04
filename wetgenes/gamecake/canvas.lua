-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require





module("wetgenes.gamecake.canvas")

base=require(...)
meta={}
meta.__index=base

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")


function create(opts)

	local canvas={}
	setmetatable(canvas,meta)
	
	canvas.cake=opts.cake
	canvas.gl=opts.gl
	
	canvas.width=opts.width or 320
	canvas.height=opts.height or 240
	
	canvas.fmt=opts.cake.canvas_fmt

	if canvas.gl then -- open
		canvas.vdat=pack.alloc(4*5*4) -- tempory vertex draw buffer		
		blit=gl_blit
	else
		canvas.grd=assert(wgrd.create(canvas.fmt,
		canvas.width, canvas.height, 1))
		blit=grd_blit
	end

	return canvas
end

grd_blit = function(canvas,img,cx,cy,ix,iy,w,h)
	assert(
			canvas.grd:blit(	canvas.cake.images.grds[img],
								cx,cy,
								ix,iy,
								w,h)
	)
end

gl_blit = function(canvas,img,cx,cy,ix,iy,w,h)

	local gl=canvas.gl

	local t=canvas.cake.images.texs[img]

	ix=ix or 0
	iy=iy or 0
	w=w or t.w
	h=h or t.h
	local tw=t.tw
	local th=t.th
	
	local cxw=cx+w
	local cyh=cy+h
	
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

	gl.VertexPointer(3,gl.FLOAT,5*4,canvas.vdat,0*0)
	gl.TexCoordPointer(2,gl.FLOAT,5*4,canvas.vdat,3*4)

	gl.BindTexture(gl.TEXTURE_2D,t.id)

	gl.DrawArrays(gl.TRIANGLE_STRIP,0,4)

end


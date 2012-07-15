-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require





module("wetgenes.gamecake.canvas")

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")


local base={}
local canvas={}
local font={}

base.font=font
base.canvas=canvas


--
-- build a simple field of view projection matrix designed to work in 2d or 3d and keep the numbers
-- easy for 2d positioning.
--
-- setting aspect to 640,480 and fov of 1 would mean at a z depth of 240 (which is y/2) then your view area would be
-- -320 to +320 in the x and -240 to +240 in the y.
--
-- fov is a tan like value (a view size inverse scalar) so 1 would be 90deg, 0.5 would be 45deg and so on
--
-- the depth parameter is only used to limit the range of the zbuffer so it covers 0 to depth
--
-- The following would be a reasonable default for a 640x480 canvas.
--
-- build_project23d(640,480,0.5,1024)
--
-- then at z=((480/2)/0.5)=480 we would have one to one pixel scale...
-- the total view area volume from there would be -320 +320 , -240 +240 , -480 +(1024-480)
--
-- canvas needs to contain width and height of the display which we use to work
-- out where to place our view such that it is always visible and keeps its aspect.
--
canvas.project23d = function(canvas,width,height,fov,depth)

	local aspect=height/width
	
	canvas.view_width=width
	canvas.view_height=height
	canvas.view_fov=fov
	canvas.view_depth=depth

	local m={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} -- defaults
	canvas.pmtx=m
	
	local f=depth
	local n=1

	local canvas_aspect=(canvas.win.height/canvas.win.width)
		
	if (canvas_aspect > (aspect) ) 	then 	-- fit width to screen
	
		m[1] = ((aspect)*1)/fov
		m[6] = -((aspect)/canvas_aspect)/fov
		
		canvas.xs=1
		canvas.ys=canvas_aspect/aspect
	else									-- fit height to screen
	
		m[1] = canvas_aspect/fov
		m[6] = -1/fov
		
		canvas.xs=aspect/canvas_aspect
		canvas.ys=1
	end
	
	
	m[11] = -(f+n)/(f-n)
	m[12] = -1

	m[15] = -2*f*n/(f-n)
	
	return m -- return the matrix but we also updated the canvas size/scale for later use
end



canvas.blit = function(canvas,t,cx,cy,ix,iy,w,h,cw,ch)

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




canvas.start = function(canvas)
	canvas.vbuf=canvas.gl.GenBuffer()
	canvas.vdat=pack.alloc(4*5*4) -- temp vertex quad draw buffer		
end

canvas.stop = function(canvas)
	if canvas.vbuf then canvas.gl.DeleteBuffer(canvas.vbuf) canvas.vbuf=nil end
	canvas.vdat=nil
end

canvas.viewport=function(canvas)
	local win=canvas.win
	local gl=canvas.gl
	
	win:info()
	gl.Viewport(0,0,win.width,win.height)
end

canvas.gl_default=function(canvas)

	local gl=canvas.gl

-- the default gl state, when we deviate from this we should restore it...

	if gl then
	
		gl.Disable(gl.LIGHTING)
		gl.Disable(gl.DEPTH_TEST)
		gl.Disable(gl.CULL_FACE)
		gl.Enable(gl.TEXTURE_2D)    
		
		gl.Color(1,1,1,1)	
		gl.EnableClientState(gl.VERTEX_ARRAY)
		gl.EnableClientState(gl.TEXTURE_COORD_ARRAY)
		gl.DisableClientState(gl.COLOR_ARRAY)
		gl.DisableClientState(gl.NORMAL_ARRAY)

	--	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
		gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
		gl.Enable(gl.BLEND)
		
	end

end




font.set = function(font,dat)
	font.dat=dat or font.dat
	font:set_size(16,0)
	font:set_xy(0,0)
end

font.set_size = function(font,size,add)
	font.size=size
	font.add=add or 0 -- clear the x space tweak
end
font.set_xy = function(font,x,y)
	font.x=x or font.x
	font.y=y or font.y
end

font.width=function(font,text)

	local font_dat=font.dat
	local s=font.size/font_dat.size
	local x=0
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]
		
		x=x+(c.add*s)+font.add
	end

	return x
end

font.draw=function(font,text)

	local x=font.x
	local y=font.y
	local font_dat=font.dat
	
	local s=font.size/font_dat.size
	
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]

		font.canvas:blit(c,x+(c.x*s),y+(c.y*s),nil,nil,c.width,c.height,c.width*s,c.height*s)

		x=x+(c.add*s)+font.add
	end
	
	font.x=x
end

function bake(opts)

	local canvas={}
	local font={}

-- fill with functions	
	for n,v in pairs(base.canvas) do canvas[n]=v end
	for n,v in pairs(base.font) do font[n]=v end

-- link together, all canvas data
	canvas.font=font
	font.canvas=canvas

-- fill in options	
	canvas.cake=opts.cake
	canvas.win=opts.cake.win
	canvas.gl=opts.gl

-- basic setup of canvas

	canvas:project23d(canvas.win.width,canvas.win.height,0.5,1024) -- some dumb defaults
	canvas:start()	
	
	font:set( canvas.cake.fonts:get(1) ) -- load default, builtin, 8x8 font
	
	return canvas
end

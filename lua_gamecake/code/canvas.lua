-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require





module("wetgenes.gamecake.canvas")

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")

local core=require("wetgenes.gamecake.core")

function bake(opts)

	local canvas={}
	local font={}
	local flat={}
	
-- link together
	canvas.font,font.canvas=font,canvas
	canvas.flat,flat.canvas=flat,canvas
-- fill in options	
	canvas.cake=opts.cake
	canvas.win=opts.cake.win
	canvas.gl=opts.gl

-- local cache
	local cake=canvas.cake
	local win=canvas.win
	local gl=canvas.gl
	local images=cake.images




canvas.blit = function(t,cx,cy,ix,iy,w,h,cw,ch)

--print("gl_blit + ",nacl.time()," ",t.filename )
	
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

	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.vbuf[0])
	gl.BufferData(gl.ARRAY_BUFFER,5*4*4,canvas.vdat,gl.DYNAMIC_DRAW)

	gl.VertexPointer(3,gl.FLOAT,5*4,0*0)
	gl.TexCoordPointer(2,gl.FLOAT,5*4,3*4)

	gl.BindTexture(gl.TEXTURE_2D,t.id)

	gl.DrawArrays(gl.TRIANGLE_STRIP,0,4)

--print("gl_blit - ",nacl.time() )

end



canvas.gl_default=function()

-- the default gl state, when we deviate from this we should restore it...

	if gl then
	
		gl.Disable(gl.LIGHTING)
		gl.EnableClientState(gl.VERTEX_ARRAY)
		gl.EnableClientState(gl.TEXTURE_COORD_ARRAY)
		gl.DisableClientState(gl.COLOR_ARRAY)
		gl.DisableClientState(gl.NORMAL_ARRAY)

		gl.Disable(gl.DEPTH_TEST)
		gl.Disable(gl.CULL_FACE)
		gl.Enable(gl.TEXTURE_2D)    
		
		gl.Color(1,1,1,1)	

		gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
		gl.Enable(gl.BLEND)
		
	end

end


-- generate functions locked to the canvas
local factory=function(canvas)

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
	canvas.project23d = function(width,height,fov,depth)
		
		local l=canvas.layout or {x=0,y=0,w=win.width,h=win.height}
		
		local aspect=height/width
		
		canvas.view_width=width
		canvas.view_height=height
		canvas.view_fov=fov
		canvas.view_depth=depth

		local m={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} -- defaults
		canvas.pmtx=m
		
		local f=depth
		local n=1

		local canvas_aspect=(l.h/l.w)
			
		if (canvas_aspect > (aspect) ) 	then 	-- fit width to screen
		
			m[1] = ((aspect)*1)/fov
			m[6] = -((aspect)/canvas_aspect)/fov
			
			canvas.x_scale=1
			canvas.y_scale=canvas_aspect/aspect
		else									-- fit height to screen
		
			m[1] = canvas_aspect/fov
			m[6] = -1/fov
			
			canvas.x_scale=aspect/canvas_aspect
			canvas.y_scale=1
		end

		canvas.x_origin=l.x+l.w/2
		canvas.y_origin=l.y+l.h/2
		canvas.x_size=l.w
		canvas.y_size=l.h

	-- we reposition with the viewport, so only need to fix the size in the matrix when using a layout.	
		
		m[11] = -(f+n)/(f-n)
		m[12] = -1

		m[15] = -2*f*n/(f-n)
		
		return m -- return the matrix but we also updated the canvas size/scale for later use
	end

-- convert raw xy coords (IE mouse win width and height) into local coords (view width and height) centered on origin
-- basically do whatever transform we came up with in project23d
	canvas.xyscale=function(x,y)

		-- centered and scaled
		x=canvas.view_width  * ( (x-canvas.x_origin) * canvas.x_scale ) / canvas.x_size
		y=canvas.view_height * ( (y-canvas.y_origin) * canvas.y_scale ) / canvas.y_size
		
		return x,y
	end

	canvas.viewport=function()

		win:info()

		local l=canvas.layout or {x=0,y=0,w=win.width,h=win.height}

		if l then -- layout mode
			gl.Viewport(l.x,win.height - (l.y+l.h) ,l.w,l.h)
		else
			gl.Viewport(0,0,win.width,win.height)
		end
		
	end


end
factory(canvas)

-- create and return a child canvas (just has its own transform cache and some functions differ)
canvas.child = function()
	local tab={}
	local meta={__index=canvas}
	setmetatable(tab,meta)
	factory(tab)
	return tab
end


font.set = function(dat)
	if dat and dat~=font.dat then -- newfont, autokill the cache?
		font.dat=dat
	end
	font.dat=dat or font.dat
	font.size=16
	font.add=0
	font.x=0
	font.y=0
	core.canvas_font_sync(font)
end

font.set_size = function(size,add)
	font.size=size
	font.add=add or 0 -- clear the x space tweak
	core.canvas_font_sync(font)
end
font.set_xy = function(x,y)
	font.x=x or font.x
	font.y=y or font.y
	core.canvas_font_sync(font)
end

font.width=function(text)

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

font.draw=function(text)

-- This needs to switch upto C func, with inline blit

	local x=font.x
	local y=font.y
	local font_dat=font.dat
	
	local s=font.size/font_dat.size
	
	local t={}
	
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]

		local vx=x+(c.x*s)
		local vxp=c.w*s
		local vy=y+(c.y*s)
		local vyp=c.h*s

		local ht=#t
		for i,v in ipairs{
			vx,		vy,		0,	c.u1,	c.v1, -- doubletap hack so we can start at any location
			vx,		vy,		0,	c.u1,	c.v1,
			vx+vxp,	vy,		0,	c.u2,	c.v1,
			vx,		vy+vyp,	0,	c.u1,	c.v2,
			vx+vxp,	vy+vyp,	0,	c.u2,	c.v2,
			vx+vxp,	vy+vyp,	0,	c.u2,	c.v2, -- doubletap hack so we can start at any location
		} do
			t[ht+i]=v
		end


		x=x+(c.add*s)+font.add
	end

	images.bind(font_dat.images[1])
	flat.tristrip("xyzuv",t)
	
	font.x=x
end

--
-- I think it is time to drop suport for gles1 in gamecake gonna need gles2...
--
if gl.fix then -- our faked fixed gles2 setup

font.draw = function(text)
	
	local p=gl.program("pos_tex")
	gl.UseProgram( p[0] )
	gl.UniformMatrix4f( p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
	gl.UniformMatrix4f( p:uniform("projection"), gl.matrix(gl.PROJECTION) )	
	gl.Uniform4f( p:uniform("color"), gl.fix.color[1],gl.fix.color[2],gl.fix.color[3],gl.fix.color[4] )

	images.bind(font.dat.images[1])
	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.vbuf[0])

	core.canvas_font_draw(font,text,p:attrib("a_vertex"),p:attrib("a_texcoord"))

end

end


flat.quad = function(x1,y1,x2,y2,x3,y3,x4,y4)

	if y4 then
		pack.save_array({
			x1,		y1,		0,		0,		0,
			x2,		y2,		0,		1,		0,
			x4,		y4,		0,		0,		1,
			x3,		y3,		0,		1,		1,
		},"f32",0,5*4,canvas.vdat)	
	else
		pack.save_array({
			x1,		y1,		0,		0,		0,
			x2,		y1,		0,		1,		0,
			x1,		y2,		0,		0,		1,
			x2,		y2,		0,		1,		1,
		},"f32",0,5*4,canvas.vdat)	
	end

	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.vbuf[0])
	gl.BufferData(gl.ARRAY_BUFFER,5*4*4,canvas.vdat,gl.DYNAMIC_DRAW)

	gl.VertexPointer(3,gl.FLOAT,5*4,0*0)
	gl.TexCoordPointer(2,gl.FLOAT,5*4,3*4)

	gl.DisableClientState(gl.TEXTURE_COORD_ARRAY)
	gl.Disable(gl.TEXTURE_2D)    
	gl.DrawArrays(gl.TRIANGLE_STRIP,0,4)
	gl.Enable(gl.TEXTURE_2D)    
	gl.EnableClientState(gl.TEXTURE_COORD_ARRAY)

end

-- tristrip is the most useful, 3 points gives us a tri
-- 4 gives us a quad, and of course you can keep going to create a strip
flat.tristrip = function(fmt,data)

-- some basic vertexformats
	local pstride
	local ptex
	local pcolor
	
	if fmt=="xyz" then -- xyz only
	
		pstride=12
	
	elseif fmt=="xyzuv" then -- xyz and texture

		pstride=20
		ptex=12
	
	elseif fmt=="xyzrgba" then -- xyz and color

		pstride=28
		pcolor=12
	
	elseif fmt=="xyzuvrgba" then -- xyz and texture and color
	
		pstride=36
		ptex=12
		pcolor=20

	end
	
	local datalen=#data
	local datasize=datalen*4 -- we need this much vdat memory
	canvas.vdat_check(datasize) -- make sure we have space in the buffer
	
	pack.save_array(data,"f32",0,datalen,canvas.vdat)	

	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.vbuf[0])
	gl.BufferData(gl.ARRAY_BUFFER,datasize,canvas.vdat,gl.DYNAMIC_DRAW)

	gl.VertexPointer(3,gl.FLOAT,pstride,0)
	
	if ptex then
		gl.TexCoordPointer(2,gl.FLOAT,pstride,ptex)
	else
		gl.DisableClientState(gl.TEXTURE_COORD_ARRAY)
		gl.Disable(gl.TEXTURE_2D)    
	end

	if pcolor then
		gl.ColorPointer(4,gl.FLOAT,pstride,pcolor)
		gl.EnableClientState(gl.COLOR_ARRAY)
	end

	gl.DrawArrays(gl.TRIANGLE_STRIP,0,datasize/pstride)
		
	if not ptex then -- revert
		gl.EnableClientState(gl.TEXTURE_COORD_ARRAY)
		gl.Enable(gl.TEXTURE_2D)    
	end
	
	if pcolor then -- revert
		gl.DisableClientState(gl.COLOR_ARRAY)
	end
	
end


-- basic setup of canvas
	canvas.vbuf=canvas.cake.buffers.create()
	canvas.vdat_size=0
	canvas.vdat_check=function(size) -- check we have enough buffer
		if canvas.vdat_size<size then
			canvas.vdat_size=size
			canvas.vdat=pack.alloc(canvas.vdat_size) -- temp draw buffer		
		end
	end
	canvas.vdat_check(1024) -- initial buffer size it may grow but this is probably more than enough
	
	canvas.cake.fonts.load(1,1) -- make sure we have loaded the 8x8 font
	font.set( canvas.cake.fonts.get(1) ) -- now use it
	
	return canvas
end

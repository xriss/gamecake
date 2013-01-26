-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")

local tcore=require("wetgenes.tardis.core")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,canvas)
		
-- link together sub parts
	local font={}
	local flat={}
	canvas.font,font.canvas=font,canvas
	canvas.flat,flat.canvas=flat,canvas

	local gl=oven.gl
	local cake=oven.cake
	local win=oven.win
	local images=cake.images

canvas.gl_default=function()

-- the default gl state, when we deviate from this we should restore it...

	if gl then

		gl.Disable(gl.DEPTH_TEST)
		gl.Disable(gl.CULL_FACE)
		
		gl.Color(1,1,1,1)	

		gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
		gl.Enable(gl.BLEND)
		
		gl.MatrixMode(gl.MODELVIEW)

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
		
		local l=canvas.layout or {}

-- make sure we have values		
		l.x=l.x or 0
		l.y=l.y or 0
		l.w=l.w or win.width
		l.h=l.h or win.height
		
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
	
-- call once to stop any null references if we recieve msgs before we resize
	canvas.project23d(640,480,0.5,1024)

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


font.draw = function(text)
	
	local p=gl.program("pos_tex")
	
	gl.UseProgram( p[0] )
	gl.UniformMatrix4f( p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
	gl.UniformMatrix4f( p:uniform("projection"), gl.matrix(gl.PROJECTION) )	
	gl.Uniform4f( p:uniform("color"), gl.cache.color )

	images.bind(font.dat.images[1])
	
	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())

	core.canvas_font_draw(font,text,p:attrib("a_vertex"),p:attrib("a_texcoord"))

end


-- should replace this with tristrip
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

	local p=gl.program("pos")
	gl.UseProgram( p[0] )
	gl.BindTexture( gl.TEXTURE_2D , 0 )

	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())
	gl.BufferData(gl.ARRAY_BUFFER,5*4*4,canvas.vdat,gl.DYNAMIC_DRAW)

	gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
	gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )
	gl.Uniform4f( p:uniform("color"), gl.cache.color )

	gl.VertexAttribPointer(p:attrib("a_vertex"),3,gl.FLOAT,gl.FALSE,5*4,0)
	gl.EnableVertexAttribArray(p:attrib("a_vertex"))

	gl.DrawArrays(gl.TRIANGLE_STRIP,0,4)

end

-- tristrip is the most useful, 3 points gives us a tri
-- 4 gives us a quad, and of course you can keep going to create a strip
flat.tristrip = function(fmt,data)

-- some basic vertexformats
	local pstride
	local ptex
	local pcolor
	local p
	if fmt=="xyz" then -- xyz only
	
		p=gl.program("pos")
		gl.UseProgram( p[0] )

		pstride=12
	
	elseif fmt=="xyzuv" then -- xyz and texture

		p=gl.program("pos_tex")
		gl.UseProgram( p[0] )

		pstride=20
		ptex=12
	
	elseif fmt=="xyzrgba" then -- xyz and color

		p=gl.program("pos_color")
		gl.UseProgram( p[0] )

		pstride=28
		pcolor=12
	
	elseif fmt=="xyzuvrgba" then -- xyz and texture and color
	
		p=gl.program("pos_tex_color")
		gl.UseProgram( p[0] )

		pstride=36
		ptex=12
		pcolor=20

	end
	
	local datalen=#data
	local datasize=datalen*4 -- we need this much vdat memory
	canvas.vdat_check(datasize) -- make sure we have space in the buffer
	
	pack.save_array(data,"f32",0,datalen,canvas.vdat)

	gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())
	gl.BufferData(gl.ARRAY_BUFFER,datasize,canvas.vdat,gl.DYNAMIC_DRAW)

	gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
	gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )
	gl.Uniform4f( p:uniform("color"), gl.cache.color )

	gl.VertexAttribPointer(p:attrib("a_vertex"),3,gl.FLOAT,gl.FALSE,pstride,0)
	gl.EnableVertexAttribArray(p:attrib("a_vertex"))
	
	if ptex then
		gl.VertexAttribPointer(p:attrib("a_texcoord"),2,gl.FLOAT,gl.FALSE,pstride,ptex)
		gl.EnableVertexAttribArray(p:attrib("a_texcoord"))
	end

	if pcolor then
		gl.VertexAttribPointer(p:attrib("a_color"),4,gl.FLOAT,gl.FALSE,pstride,pcolor)
		gl.EnableVertexAttribArray(p:attrib("a_color"))
	end

	gl.DrawArrays(gl.TRIANGLE_STRIP,0,datasize/pstride)
		
	
end



function canvas.delete_vbs()
	for i,v in ipairs(canvas.vbs) do
		gl.DeleteBuffer(v)
	end
	canvas.vbs={}
	canvas.vbi=1
end

function canvas.reuse_vbs()
	canvas.vbi=1
end

function canvas.get_vb()
	local vb=canvas.vbs[canvas.vbi]
	if not vb then
		vb=gl.GenBuffer()
		canvas.vbs[canvas.vbi]=vb
	end
	canvas.vbi=canvas.vbi+1
	return vb
end


function canvas.start()
end
function canvas.stop()
	canvas.delete_vbs()
end
function canvas.draw()
	if canvas.vbi_flop then
		canvas.reuse_vbs()
	end
	canvas.vbi_flop=not canvas.vbi_flop
	cake.sheets.UseSheet=nil
end

-- basic setup of canvas
	canvas.vbs={}
	canvas.vbi=1
	
	canvas.vdat_size=0
	canvas.vdat_check=function(size) -- check we have enough buffer
		if canvas.vdat_size<size then
			canvas.vdat_size=size
			canvas.vdat=pack.alloc(canvas.vdat_size) -- temp draw buffer		
		end
	end
	canvas.vdat_check(1024) -- initial buffer size it may grow but this is probably more than enough
	
	cake.fonts.load(1,1) -- make sure we have loaded the 8x8 font
	font.set( cake.fonts.get(1) ) -- now use it
	
	return canvas
end

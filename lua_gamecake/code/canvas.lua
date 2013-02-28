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

	gl.Disable(gl.DEPTH_TEST)
	gl.Disable(gl.CULL_FACE)
	
	gl.Color(1,1,1,1)	

	gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
	gl.Enable(gl.BLEND)
	
	gl.MatrixMode(gl.MODELVIEW)

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

font.xindex=function(text,px)
	if px<0 then return 1 end
	
	local font_dat=font.dat
	local s=font.size/font_dat.size
	local x=0
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]
		
		x=x+(c.add*s)+font.add
		
		if x>=px then return i end
	end

	return #text
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

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")


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
	local fonts=cake.fonts
	local images=cake.images
	local buffers=cake.buffers

-- prefer shader that discard pixels with low alpha < 0.25
	canvas.discard_low_alpha=false

canvas.gl_default=function()

-- the default gl state, when we deviate from this we should restore it...

	gl.PixelStore(gl.PACK_ALIGNMENT,1)
	gl.PixelStore(gl.UNPACK_ALIGNMENT,1) -- the grd code expects fully packed bitmaps

	gl.Disable(gl.DEPTH_TEST)
	gl.Disable(gl.CULL_FACE)
	
	gl.Color(1,1,1,1)	

	gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
	gl.Enable(gl.BLEND)
	
	if not canvas.NO_GL_PROGRAM_POINT_SIZE then
		gl.Enable(0x8642) -- #define GL_PROGRAM_POINT_SIZE 0x8642
		if gl.GetError() == gl.INVALID_ENUM then    -- do not repeat error
			canvas.NO_GL_PROGRAM_POINT_SIZE=true
		end
	end
		
	gl.MatrixMode(gl.MODELVIEW)

end


font.set = function(dat)
	do local t=type(dat) if t=="string" or t=="number" then dat=fonts.get(dat) end end
	if dat and dat~=font.dat then -- newfont, autokill the cache?
		font.dat=dat
	end
	font.dat=dat or font.dat
	font.size=16
	font.add=0
	font.x=0
	font.y=0
--	if gl.patch_functions_method~="disable" then
		core.canvas_font_sync(font)
--	end
end

font.set_size = function(size,add)
	font.size=size
	font.add=add or 0 -- clear the x space tweak
--	if gl.patch_functions_method~="disable" then
		core.canvas_font_sync(font)
--	end
end
font.set_xy = function(x,y)
	font.x=x or font.x
	font.y=y or font.y
--	if gl.patch_functions_method~="disable" then
		core.canvas_font_sync(font)
--	end
end

font.xindex=function(text,px,dat,size,add)
	if px<0 then return 1 end
	
	local font_dat=dat or font.dat
	local s=(size or font.size)/font_dat.size
	local x=0
	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]
		
		x=x+(c.add*s)+(add or font.add)
		
		if x>=px then return i-1 end
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


function font.wrap(text,opts)
	local w=opts.w
	
	local ls=wstr.split_whitespace(text)
	local t={}
	
	local wide=0
	local line={}
	
	local function newline()
		t[#t+1]=table.concat(line," ") or ""
		wide=0
		line={}
	end
	
	for i,v in ipairs(ls) do
	
		if v:find("%s") then -- just white space
		
			for i,v in string.gfind(v,"\n") do -- keep newlines
				newline()
			end
		
		else -- a normal word
		
			local fw=font.width(v)
			if #line>0 then wide=wide+font.width(" ") end

			if wide + fw > w then -- split
				newline()
			end
			
			line[#line+1]=v
			wide=wide+fw
			
		end
	end
	if wide~=0 then newline() end -- final newline
	
	return t
end

font.cache_begin = function()
--print("font begine",tostring(font.dat))
	local t={}
	local old=font.cache
	font.cache=t
	return function()
		local r,g,b,a=gl.color_get_rgba()
		gl.Color(1,1,1,1)	
		for d,v in pairs(t) do -- multifonts
--print("font draw",#v,tostring(d))
			if v[1] then
				images.bind(d.images[1])
				flat.tristrip("rawuvrgba",v)
			end
		end
		gl.Color(r,g,b,a)
		font.cache=old
	end
end

font.cache_predraw = function(text)

	local font_dat=font.dat
	local font_cache=font.cache[ font_dat ] or {}
	font.cache[ font_dat ]=font_cache
	
	local s=font.size/font_dat.size
	local x=font.x
	local y=font.y

	local insert=function(a,b,c,d,e,f,g,h,i)
		local idx=#font_cache
		local fc=font_cache
		fc[idx+1]=a	fc[idx+2]=b	fc[idx+3]=c
		fc[idx+4]=d	fc[idx+5]=e
		fc[idx+6]=f	fc[idx+7]=g	fc[idx+8]=h	fc[idx+9]=i			
	end

	local r,g,b,a=gl.color_get_rgba()

	for i=1,#text do
	
		local cid=text:byte(i)
		local c=font_dat.chars[cid] or font_dat.chars[32]
				
		local vx=x+(c.x*s);
		local vxp=c.w*s;
		local vy=y+(c.y*s);
		local vyp=c.h*s;

		local v1=gl.apply_modelview( {vx,vy,0,1} )
		local v2=gl.apply_modelview( {vx+vxp,vy,0,1} )
		local v3=gl.apply_modelview( {vx,vy+vyp,0,1} )
		local v4=gl.apply_modelview( {vx+vxp,vy+vyp,0,1} )
		
		insert(	v1[1],v1[2],v1[3],	c.u1,c.v1,	r,g,b,a	)
		insert(	v1[1],v1[2],v1[3],	c.u1,c.v1,	r,g,b,a	)
		insert(	v2[1],v2[2],v2[3],	c.u2,c.v1,	r,g,b,a	)
		insert(	v3[1],v3[2],v3[3],	c.u1,c.v2,	r,g,b,a	)
		insert(	v4[1],v4[2],v4[3],	c.u2,c.v2,	r,g,b,a	)
		insert(	v4[1],v4[2],v4[3],	c.u2,c.v2,	r,g,b,a	)
		
		x=x+(c.add*s)+font.add
	end


end


font.draw = function(text)

	if font.cache then
		return font.cache_predraw(text)
	end


	local dataraw,datalen=core.canvas_font_draw(font,text)
	if datalen/5>=1 then -- need something to draw
		images.bind(font.dat.images[1])
		local it=flat.array_predraw({fmt="posuv",dataraw=dataraw,datalen=datalen,array=gl.TRIANGLES})
		it.draw(cb)
	end

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

-- this allows us to prepare vertex buffers, then draw at anytime
-- just call it.draw on the returned table.
flat.array_predraw = function(it) -- pass in fmt,data,progname,vb=-1 in here

-- some basic vertexformats
	local pstride
	local pnrm
	local ptex
	local pcolor
	local pmat
	local ptans
	local pbone
	local p
	local def_progname
	local fmt=it.fmt
	if fmt=="xyz" or fmt=="pos" then -- xyz or pos only
	
		def_progname=fmt:sub(1,3)
		pstride=12
	
	elseif fmt=="xyznrm" or fmt=="posnrm" then -- xyz and normal (so we may light the thing)

		def_progname=fmt:sub(1,3).."_normal"

		pstride=24
		pnrm=12
	
	elseif fmt=="xyznrmuv" or fmt=="posnrmuv" then -- xyz and normal and texture

		def_progname=fmt:sub(1,3).."_normal_tex"

		pstride=32
		pnrm=12
		ptex=24
	
	elseif fmt=="xyznrmuvm" or fmt=="posnrmuvm" then -- xyz and normal and texture and  material id

		def_progname=fmt:sub(1,3).."_normal_tex_mat"

		pstride=36
		pnrm=12
		ptex=24
		pmat=32
	
	elseif fmt=="xyznrmuvmbone" or fmt=="posnrmuvmbone" then -- xyz and normal and texture and  material id and bones

		def_progname=fmt:sub(1,3).."_normal_tex_mat_bone"

		pstride=52
		pnrm=12
		ptex=24
		pmat=32
		pbone=36

	elseif fmt=="xyznrmuvmtansbone" or fmt=="posnrmuvmtansbone" then -- xyz and normal and texture and  material id and tangent and bones

		def_progname=fmt:sub(1,3).."_normal_tex_mat_tans_bone"

		pstride=68
		pnrm=12
		ptex=24
		pmat=32
		ptans=36
		pbone=52

	elseif fmt=="xyznrmuvmtans" or fmt=="posnrmuvmtans" then -- xyz and normal and texture and  material id and tangent

		def_progname=fmt:sub(1,3).."_normal_tex_mat_tans"

		pstride=52
		pnrm=12
		ptex=24
		pmat=32
		ptans=36

	elseif fmt=="xyzuv" or fmt=="posuv" then -- xyz and texture

		def_progname=fmt:sub(1,3).."_tex"

		pstride=20
		ptex=12
	
	elseif fmt=="xyzrgba" or fmt=="posrgba" then -- xyz and color

		def_progname=fmt:sub(1,3).."_color"

		pstride=28
		pcolor=12
	
	elseif fmt=="xyzuvrgba" or fmt=="posuvrgba" then -- xyz and texture and color
	
		def_progname=fmt:sub(1,3).."_tex_color"

		pstride=36
		ptex=12
		pcolor=20

	elseif fmt=="rawuvrgba" then -- raw-xyz and texture and color
	
		def_progname="raw_tex_color"

		pstride=36
		ptex=12
		pcolor=20

	elseif fmt=="rawuv" then -- raw-xyz and texture and color
	
		def_progname="raw_tex"

		pstride=20
		ptex=12

	end
	
	if canvas.discard_low_alpha then -- try not to break the zbuffer
		def_progname=def_progname.."_discard"
	end

	
	local data=it.data
	local datalen=it.datalen or #data
	local datasize=datalen*4 -- we need this much vdat memory
	canvas.vdat_check(datasize) -- make sure we have space in the buffer
	
	if it.vb then
		if type(it.vb)~="table" then -- need to create
			it.vb=buffers.create({
				start=function(vb)
					vb:bind()
					pack.save_array(data,"f32",0,datalen,canvas.vdat)
					gl.BufferData(gl.ARRAY_BUFFER,datasize,canvas.vdat,gl.STATIC_DRAW)
				end,
			})
		end
	end


	it.draw=function(cb,progname)
	
		local p=gl.program(progname or it.progname or def_progname)
		gl.UseProgram( p[0] )

		if it.vb then -- use a precached buffer
			it.vb:bind()
		elseif it.dataraw then -- use prebuilt data
			gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())
			gl.BufferData(gl.ARRAY_BUFFER,datasize,it.dataraw,gl.DYNAMIC_DRAW)
		else
			pack.save_array(data,"f32",0,datalen,canvas.vdat)
			gl.BindBuffer(gl.ARRAY_BUFFER,canvas.get_vb())
			gl.BufferData(gl.ARRAY_BUFFER,datasize,canvas.vdat,gl.DYNAMIC_DRAW)
		end

		gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
		gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )
		gl.Uniform4f( p:uniform("color"), gl.cache.color )

		gl.VertexAttribPointer(p:attrib("a_vertex"),3,gl.FLOAT,gl.FALSE,pstride,0)
		gl.EnableVertexAttribArray(p:attrib("a_vertex"))
		
		if pnrm then
			gl.VertexAttribPointer(p:attrib("a_normal"),3,gl.FLOAT,gl.FALSE,pstride,pnrm)
			gl.EnableVertexAttribArray(p:attrib("a_normal"))
		end

		if ptex then
			gl.VertexAttribPointer(p:attrib("a_texcoord"),2,gl.FLOAT,gl.FALSE,pstride,ptex)
			gl.EnableVertexAttribArray(p:attrib("a_texcoord"))
		end

		if pcolor then
			gl.VertexAttribPointer(p:attrib("a_color"),4,gl.FLOAT,gl.FALSE,pstride,pcolor)
			gl.EnableVertexAttribArray(p:attrib("a_color"))
		end

		if pmat then
			gl.VertexAttribPointer(p:attrib("a_matidx"),1,gl.FLOAT,gl.FALSE,pstride,pmat)
			gl.EnableVertexAttribArray(p:attrib("a_matidx"))
		end
		
		if ptans then
			gl.VertexAttribPointer(p:attrib("a_tangent"),4,gl.FLOAT,gl.FALSE,pstride,ptangent)
			gl.EnableVertexAttribArray(p:attrib("a_tangent"))
		end

		if pbone then
			gl.VertexAttribPointer(p:attrib("a_bone"),4,gl.FLOAT,gl.FALSE,pstride,pbone)
			gl.EnableVertexAttribArray(p:attrib("a_bone"))
		end

		if cb then cb(p) end -- callback to fill in more uniforms before drawing

		local cc=datasize/pstride
		if cc>0 then
			gl.DrawArrays( it.array or gl.TRIANGLE_STRIP,0,cc)
		end
	end

	return it
end
flat.tristrip_predraw=flat.array_predraw

-- tristrip is the most useful, 3 points gives us a tri
-- 4 gives us a quad, and of course you can keep going to create a strip
flat.tristrip = function(fmt,data,progname,cb)
	if #data > 0 then
		local it=flat.array_predraw({fmt=fmt,data=data,progname=progname,array=gl.TRIANGLE_STRIP})
		it.draw(cb)
	end
end

flat.lines = function(fmt,data,progname,cb)
	if #data > 0 then
		local it=flat.array_predraw({fmt=fmt,data=data,progname=progname,array=gl.LINES})
		it.draw(cb)
	end
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
--print(canvas.vbi)
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

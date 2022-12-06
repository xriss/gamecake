--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local grd=require("wetgenes.grd")
local pack=require("wetgenes.pack")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,sheets)

local base_sheet={}
local meta_sheet={__index=base_sheet}

	sheets=sheets or {}
		
	sheets.data={}
	
	local opts=oven.opts
	local cake=oven.cake
	local gl=oven.gl
	local images=cake.images
		
sheets.get=function(id)
	return id and sheets.data[id]
end

sheets.set=function(d,id)
	if not id then id = #sheets.data+1 end
	sheets.data[id]=d
end

sheets.start=function()
	for i,v in pairs(sheets.data) do -- refresh image data after a stop
--		if v.img_id and not v.img then
--			v.img=images.get(v.img_id)
			if v.img and not v.vbuf then
				v:build_vbuf()
			end
--		end
	end
	
end

sheets.stop=function()
	for i,v in pairs(sheets.data) do -- forget everything
--		v.img=nil
		v:free_vbuf()
	end
end


function sheets.batch_draw()
	for i,v in pairs(sheets.data) do
		if v.batch and #v.batch>0 then
			v:batch_draw()
		end
	end
end

function sheets.batch_start()
	for i,v in pairs(sheets.data) do
		v:batch_start()
	end
end

function sheets.batch_stop()
	for i,v in pairs(sheets.data) do
		v:batch_stop()
	end
end



sheets.create=function(id)

	local sheet=sheets.get(id)
	if sheet then return sheet end
	
	sheet={}
	sheet.sheets=sheets
	setmetatable(sheet,meta_sheet)

	sheets.set(sheet,id)

	return sheet
end

sheets.createimg=function(id)
	return sheets.create(id):setimg(id)
end

--
-- Load images and chop them up
--
sheets.loads_and_chops=function(tab)

	for i,v in ipairs(tab) do
		images.load(v[1],v[1])
		local img=sheets.createimg(v[1])
		img:chop(v[2],v[3],v[4],v[5],v[6],v[7])
		
	end

end


function base_sheet.setimg(sheet,img_id)

	sheet.img_id=img_id
	sheet.img=images.get(sheet.img_id)

	return sheet
end


function base_sheet.chop(sheet,hx,hy,ox,oy,bx,by)

	hx=hx or 1 -- default to full size chop
	hy=hy or 1
	
	if hx>1 then hx=1/hx end -- auto invert so we can say eg 4,4 instead of 1/4,1/4
	if hy>1 then hy=1/hy end -- these should never be >1 so it's safe to do

	ox=ox or hx/2	-- default is center if no handle is given
	oy=oy or hy/2

	bx=bx or 0 -- and of course no border by deafult
	by=by or 0
	
-- coords are fractions of image, so the image can scale up/down but the code remains constant
	bx=bx*sheet.img.width
	by=by*sheet.img.height
	hx=hx*sheet.img.width
	hy=hy*sheet.img.height
	ox=ox*sheet.img.width
	oy=oy*sheet.img.height
	
	sheet.cx=math.floor( sheet.img.width / hx )
	sheet.cy=math.floor( sheet.img.height / hy )
	sheet.cc=sheet.cx*sheet.cy
	
	for iy=1,sheet.cy do
		for ix=1,sheet.cx do
			local i=(iy-1)*sheet.cx + ix
			local c={}
			sheet[i]=c
			c.sheet=sheet
			c.px=(ix-1)*hx+bx
			c.py=(iy-1)*hy+by
			c.hx=hx-bx*2
			c.hy=hy-by*2
			c.ox=ox-bx
			c.oy=oy-by
		end
	end
	
	sheet:build_vbuf()
	
	return sheet
end

-- free the sheets vertex buffer
function base_sheet.free_vbuf(sheet)

	if sheet.vbuf then -- free any previously allocated buffer
		gl.DeleteBuffer(sheet.vbuf)
	end
	
	sheet.vdat=nil
	sheet.vbuf=nil

	return sheet
end

-- buildvertex buffer containing all sprites in the sheet
function base_sheet.build_vbuf(sheet)

	sheet:free_vbuf()
	
	sheet.vdat=pack.alloc(#sheet*4*5*4) -- 4 vertexs per sprite, 5 floats per vertex
	
	local tw=sheet.img.texture_width -- hacks for cards that do not suport non power of two texture sizes
	local th=sheet.img.texture_height -- we just use a part of this bigger texture

--print("sheet",#sheet,#sheet*4*5*4,(#sheet-1)*4*5*4,5*4)

	for i,v in ipairs(sheet) do
		
--		local cxw=cx+cw
--		local cyh=cy+ch
		
		local ixw=(v.px+v.hx)/tw
		local iyh=(v.py+v.hy)/th
		local ix=v.px/tw
		local iy=v.py/th

--print((i-1)*4*5*4,5*4)
		pack.save_array({
			0-v.ox,		0-v.oy,		0,		ix,		iy,
			v.hx-v.ox,	0-v.oy,		0,		ixw,	iy,
			0-v.ox,		v.hy-v.oy,	0,		ix,		iyh,
			v.hx-v.ox,	v.hy-v.oy,	0,		ixw,	iyh,
		},"f32",(i-1)*4*5*4,5*4,sheet.vdat)	
	
	end
	
	sheet.vbuf=gl.GenBuffer()
	gl.BindBuffer(gl.ARRAY_BUFFER,sheet.vbuf)
	gl.BufferData(gl.ARRAY_BUFFER,#sheet*4*5*4,sheet.vdat,gl.STATIC_DRAW)

	return sheet
end

function base_sheet.bind(sheet)
	images.bind(sheet.img)
end

function base_sheet.batch_draw(sheet)

	cake.canvas.flat.tristrip("rawuvrgba",sheet.batch,nil,function(p)
			gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			images.bind(sheet.img)
			gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
	end)

end

function base_sheet.batch_start(sheet)
	sheet.batch={}
	sheet.batch_record=true
end

function base_sheet.batch_stop(sheet)
	sheet.batch_record=false
end

function base_sheet.draw(sheet,i,px,py,rz,sx,sy,zf)

--	if not sheet.vbuf then base_sheet.build_vbuf(sheet) end -- may need to rebuild?

	zf=zf or 0 -- allow z fix hacks

	if i<1 or i>#sheet then error("sheet index out of bounds "..i.." of "..#sheet) end
	
	if sheet.batch_record then -- cache for later drawing (rz is currently ignored, soz)
		local v=sheet[i]
		
		local tw=sheet.img.texture_width -- hacks for cards that do not suport non power of two texture sizes
		local th=sheet.img.texture_height -- we just use a part of this bigger texture

		local ixw=(v.px+v.hx)/tw
		local iyh=(v.py+v.hy)/th
		local ix=v.px/tw
		local iy=v.py/th

		
		if sx then
			sy=sy or sx
			sx=sx/v.hx
			sy=sy/v.hy
		else
			sx=1
			sy=1
		end
		
		local ox=v.ox*sx
		local oy=v.oy*sy
		local hx=v.hx*sx
		local hy=v.hy*sy
		
		local s=-math.sin(math.pi*rz/180)
		local c=math.cos(math.pi*rz/180)

		local r,g,b,a=gl.color_get_rgba()
		local v1=gl.apply_modelview( {px-c*(ox)-s*(oy),			py+s*(ox)-c*(oy),		zf,1} )
		local v2=gl.apply_modelview( {px+c*(hx-ox)-s*(oy),		py-s*(hx-ox)-c*(oy),	zf,1} )
		local v3=gl.apply_modelview( {px-c*(ox)+s*(hy-oy),		py+s*(ox)+c*(hy-oy),	zf,1} )
		local v4=gl.apply_modelview( {px+c*(hx-ox)+s*(hy-oy),	py-s*(hx-ox)+c*(hy-oy),	zf,1} )


		local t=
		{
			v1[1],	v1[2],	v1[3],		ix,		iy,			r,g,b,a,
			v1[1],	v1[2],	v1[3],		ix,		iy,			r,g,b,a,
			v2[1],	v2[2],	v2[3],		ixw,	iy,			r,g,b,a,
			v3[1],	v3[2],	v3[3],		ix,		iyh,		r,g,b,a,
			v4[1],	v4[2],	v4[3],		ixw,	iyh,		r,g,b,a,
			v4[1],	v4[2],	v4[3],		ixw,	iyh,		r,g,b,a,
		}

--[[
		local t=
		{
			px-c*(ox)-s*(oy),		py+s*(ox)-c*(oy),		zf,		ix,		iy,
			px-c*(ox)-s*(oy),		py+s*(ox)-c*(oy),		zf,		ix,		iy,
			px+c*(hx-ox)-s*(oy),	py-s*(hx-ox)-c*(oy),	zf,		ixw,	iy,
			px-c*(ox)+s*(hy-oy),	py+s*(ox)+c*(hy-oy),	zf,		ix,		iyh,
			px+c*(hx-ox)+s*(hy-oy),	py-s*(hx-ox)+c*(hy-oy),	zf,		ixw,	iyh,
			px+c*(hx-ox)+s*(hy-oy),	py-s*(hx-ox)+c*(hy-oy),	zf,		ixw,	iyh,
		}
]]
		
		for i,v in ipairs(t) do
			sheet.batch[ #sheet.batch+1 ]=v
		end
		
		return sheet
	end

	if px then
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()
		gl.Translate(px,py,0)
		if rz then
			gl.Rotate(rz,0,0,1)
		end
		if sx then
			sy=sy or sx
			gl.Scale(sx/sheet[i].hx,sy/sheet[i].hy,1) -- fixed, ignore the base size of image when we scale. So size is absolute
		end
	end
	
	
	local p

	if sheets.discard_low_alpha then -- try not to break the zbuffer
		p=gl.program("pos_tex_discard")
	else
		p=gl.program("pos_tex")
	end
	
	gl.BindBuffer(gl.ARRAY_BUFFER,sheet.vbuf)
	
	local vertexarray
	if gl.GenVertexArray then
		vertexarray=gl.GenVertexArray()
		gl.BindVertexArray(vertexarray)
	end

	gl.UseProgram( p[0] )
	gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
	gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )

	gl.VertexAttribPointer(p:attrib("a_vertex"),3,gl.FLOAT,gl.FALSE,5*4,0)
	gl.EnableVertexAttribArray(p:attrib("a_vertex"))
		
	gl.VertexAttribPointer(p:attrib("a_texcoord"),2,gl.FLOAT,gl.FALSE,5*4,3*4)
	gl.EnableVertexAttribArray(p:attrib("a_texcoord"))
	
	images.bind(sheet.img)
--	gl.BindTexture(gl.TEXTURE_2D,sheet.img.gl)

	gl.Uniform4f(p:uniform("color"), gl.cache.color )
	gl.core.DrawArrays(gl.TRIANGLE_STRIP,(i-1)*4,4)

	if gl.GenVertexArray then
		gl.BindVertexArray(0)
		gl.DeleteVertexArray(vertexarray)
	end

	if px then
		gl.PopMatrix()
	end
	
--	gl.fix.cache.UseProgram=p[0]

	return sheet

end

	
	return sheets
end


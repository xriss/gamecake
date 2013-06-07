-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local grd=require("wetgenes.grd")
local pack=require("wetgenes.pack")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,sheets)

local base_sheet={}
local meta_sheet={__index=base_sheet}

	local sheets={}
		
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
		if v.img_id and not v.img then
			v.img=images.get(v.img_id)
			if v.img then
				v:build_vbuf()
			end
		end
	end
	
end

sheets.stop=function()
	for i,v in pairs(sheets.data) do -- forget everything
		v.img=nil
		v:free_vbuf()
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

	hx=hx or 1
	hy=hy or 1

	ox=ox or 0
	oy=oy or 0

	bx=bx or 0
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

function base_sheet.batch_draw(sheet)

	gl.BindTexture(gl.TEXTURE_2D,sheet.img.id)	
	cake.canvas.flat.tristrip("xyzuv",sheet.batch)

end

function base_sheet.batch_start(sheet)
	sheet.batch={}
	sheet.batch_record=true
end

function base_sheet.batch_stop(sheet)
	sheet.batch_record=false
end

function base_sheet.draw(sheet,i,px,py,rz,sx,sy,zf)

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

		local t=
		{
			px-ox,		py-oy,		zf,		ix,		iy,
			px-ox,		py-oy,		zf,		ix,		iy,
			px+hx-ox,	py-oy,		zf,		ixw,	iy,
			px-ox,		py+hy-oy,	zf,		ix,		iyh,
			px+hx-ox,	py+hy-oy,	zf,		ixw,	iyh,
			px+hx-ox,	py+hy-oy,	zf,		ixw,	iyh,
		}
		
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
	
	
	local p=gl.program("pos_tex")
	
	gl.BindBuffer(gl.ARRAY_BUFFER,sheet.vbuf)

	gl.UseProgram( p[0] )
	gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
	gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )

	gl.VertexAttribPointer(p:attrib("a_vertex"),3,gl.FLOAT,gl.FALSE,5*4,0)
	gl.EnableVertexAttribArray(p:attrib("a_vertex"))
		
	gl.VertexAttribPointer(p:attrib("a_texcoord"),2,gl.FLOAT,gl.FALSE,5*4,3*4)
	gl.EnableVertexAttribArray(p:attrib("a_texcoord"))
	
	gl.BindTexture(gl.TEXTURE_2D,sheet.img.id)
	gl.Uniform4f(p:uniform("color"), gl.cache.color )
	gl.core.DrawArrays(gl.TRIANGLE_STRIP,(i-1)*4,4)

	if px then
		gl.PopMatrix()
	end
	
--	gl.fix.cache.UseProgram=p[0]

	return sheet

end

	
	return sheets
end


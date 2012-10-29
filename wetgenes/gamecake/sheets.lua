-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.sheets")


local grd=require("wetgenes.grd")
local pack=require("wetgenes.pack")



function bake(opts)

local base_sheet={}
local meta_sheet={__index=base_sheet}

	local sheets={}
	
	sheets.cake=opts.cake
	sheets.gl=opts.gl
	
	sheets.data={}
	
	local gl=sheets.gl
	local cake=sheets.cake
	local images=cake.images
	
sheets.get=function(id)
	return sheets.data[id]
end

sheets.set=function(d,id)
	sheets.data[id]=d
end

sheets.start=function()
	for i,v in pairs(sheets.data) do -- refresh image data after a stop
		if v.img_id and not v.img then
			v.img=images.get(v.img_id)
			v:build_vbuf()
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
		img:chop(v[2],v[3],v[4],v[5])
	end

end


function base_sheet.setimg(sheet,img_id)

	sheet.img_id=img_id
	sheet.img=images.get(sheet.img_id)

	return sheet
end


function base_sheet.chop(sheet,hx,hy,ox,oy)

	hx=hx or 1
	hy=hy or 1

	ox=ox or 0
	oy=oy or 0
	
-- coords are fractions of image, so the image can scale up/down but the code remains constant
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
			c.px=(ix-1)*hx
			c.py=(iy-1)*hy
			c.hx=hx
			c.hy=hy
			c.ox=ox
			c.oy=oy
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
	
	for i,v in ipairs(sheet) do
		
--		local cxw=cx+cw
--		local cyh=cy+ch
		
		local ixw=(v.px+v.hx)/tw
		local iyh=(v.py+v.hy)/th
		local ix=v.px/tw
		local iy=v.py/th

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


-- draw one sprite, normal lua indexs, so first sprite is 1 not 0
function base_sheet.draw(sheet,i,px,py,rz,sx,sy)
		
	if px then
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()
		gl.Translate(px,py,0)
		if rz then
			gl.Rotate(rz,0,0,1)
		end
		if sx then
			sy=sy or sx
			gl.Scale(sx/sheet[i].hx,sy/sheet[i].hy,1) -- fixed, ignore base size of image?
		end
	end
	
	gl.BindBuffer(gl.ARRAY_BUFFER,sheet.vbuf)

	gl.VertexPointer(   3, gl.FLOAT, 5*4, 0*4 )
	gl.TexCoordPointer( 2, gl.FLOAT, 5*4, 3*4 )

	gl.BindTexture(gl.TEXTURE_2D,sheet.img.id)

	gl.DrawArrays(gl.TRIANGLE_STRIP,(i-1)*4,4)

	if px then
		gl.PopMatrix()
	end
	
	return sheet
end


	
	return sheets
end


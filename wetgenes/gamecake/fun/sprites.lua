--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,sprites)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

sprites.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return sprites
end

sprites.setup=function()

	sprites.load()

	return sprites
end

sprites.create=function(it,opts)
	it.screen=it.system.components.screen -- system will have been passed in
	it.opts=opts
	it.component="sprites"
	it.name=opts.name

	it.sprite_hx=it.opts.sprite_size and it.opts.sprite_size[1] or 8
	it.sprite_hy=it.opts.sprite_size and it.opts.sprite_size[2] or 8

	it.bitmap_hx=it.opts.bitmap_size and it.opts.bitmap_size[1] or 16
	it.bitmap_hy=it.opts.bitmap_size and it.opts.bitmap_size[2] or 16
	

	it.setup=function(opts)
		
		it.px=0 -- display x offset 1 is a single char wide
		it.py=0 -- display y offset 1 is a single char high
		
		it.bitmap_grd  =wgrd.create("U8_RGBA", it.sprite_hx*it.bitmap_hx , it.sprite_hy*it.bitmap_hy , 1)

		it.bitmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)
		
		it.list={}

	end

	it.clean=function()
		if it.bitmap_tex then
			gl.DeleteTexture( it.bitmap_tex )
			it.bitmap_tex=nil
		end
	end
	
	it.list_reset=function()
		it.list={}
	end
	it.list_add=function(v,idx)

		v.idx=(idx or v.idx or (#it.list+1) )
		it.list[ v.idx ]=v
		
		v.cx=v.cx or 0
		v.cy=v.cy or 0

		v.ox=v.ox or it.sprite_hx/2
		v.oy=v.oy or it.sprite_hy/2
		
		v.px=v.px or 0
		v.py=v.py or 0

		v.rz=v.rz or 0

		v.sx=v.sx or 1
		v.sy=v.sy or 1

		v.zf=v.zf or 0

		v.r=v.r or 1
		v.g=v.g or 1
		v.b=v.b or 1
		v.a=v.a or 1
		
	end

	it.update=function()
	end
	
	it.draw=function()

		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			it.bitmap_grd.width,
			it.bitmap_grd.height,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			it.bitmap_grd.data )

		
		local batch={}
		for idx,v in pairs(it.list) do

			local ixw=(v.cx+1)/it.bitmap_hx
			local iyh=(v.cy+1)/it.bitmap_hy
			local ix=v.cx/it.bitmap_hx
			local iy=v.cy/it.bitmap_hy
			
			local ox=(v.ox)*(v.sx)
			local oy=(v.oy)*(v.sy)
			local hx=it.sprite_hx*(v.sx)
			local hy=it.sprite_hy*(v.sy)
			
			local s=-math.sin(math.pi*(v.rz)/180)
			local c= math.cos(math.pi*(v.rz)/180)

			local v1=gl.apply_modelview( {v.px-c*(ox)-s*(oy),			v.py+s*(ox)-c*(oy),			v.zf,1} )
			local v2=gl.apply_modelview( {v.px+c*(hx-ox)-s*(oy),		v.py-s*(hx-ox)-c*(oy),		v.zf,1} )
			local v3=gl.apply_modelview( {v.px-c*(ox)+s*(hy-oy),		v.py+s*(ox)+c*(hy-oy),		v.zf,1} )
			local v4=gl.apply_modelview( {v.px+c*(hx-ox)+s*(hy-oy),		v.py-s*(hx-ox)+c*(hy-oy),	v.zf,1} )

			local t=
			{
				v1[1],	v1[2],	v1[3],		ix,		iy,			v.r,v.g,v.b,v.a,
				v1[1],	v1[2],	v1[3],		ix,		iy,			v.r,v.g,v.b,v.a,
				v2[1],	v2[2],	v2[3],		ixw,	iy,			v.r,v.g,v.b,v.a,
				v3[1],	v3[2],	v3[3],		ix,		iyh,		v.r,v.g,v.b,v.a,
				v4[1],	v4[2],	v4[3],		ixw,	iyh,		v.r,v.g,v.b,v.a,
				v4[1],	v4[2],	v4[3],		ixw,	iyh,		v.r,v.g,v.b,v.a,
			}
			
			local l=#batch for i,v in ipairs(t) do batch[ l+i ]=v end

		end

		flat.tristrip("rawuvrgba",batch)

	end

	return it
end

	return sprites
end



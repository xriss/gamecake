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

	it.tiles=assert(it.system.components[it.opts.tiles or "tiles"]) -- find tile bitmap by name

	it.setup=function(opts)
		
		it.px=0 -- display x offset 1 is a single tile wide
		it.py=0 -- display y offset 1 is a single tile high
				
		it.list={}

	end

	it.clean=function()
	end
	
	it.list_reset=function()
		it.list={}
	end
	it.list_add=function(v,idx)

		v.idx=(idx or v.idx or (#it.list+1) )
		it.list[ v.idx ]=v
		
		v.hx=v.hx or v.h or it.tiles.tile_hx
		v.hy=v.hy or v.h or it.tiles.tile_hy

		v.tx=v.tx or (           ( (v.t or 0)%256 ) )--* math.ceil(v.hx/it.tiles.tile_hx) )
		v.ty=v.ty or ( math.floor( (v.t or 0)/256 ) )--* math.ceil(v.hy/it.tiles.tile_hy) )

		v.ox=v.ox or v.hx/2
		v.oy=v.oy or v.hy/2
		
		v.px=v.px or 0
		v.py=v.py or 0

		v.rz=v.rz or 0

		v.sx=v.sx or v.s or 1
		v.sy=v.sy or v.s or 1

		v.zf=v.zf or 0

		v.r=v.r or 1
		v.g=v.g or 1
		v.b=v.b or 1
		v.a=v.a or 1
		
	end

	it.update=function()
	end
	
	it.draw=function()
		
		local batch={}
		for idx,v in pairs(it.list) do

			local ixw=(v.tx+(v.hx/it.tiles.tile_hx))/it.tiles.bitmap_hx
			local iyh=(v.ty+(v.hy/it.tiles.tile_hy))/it.tiles.bitmap_hy
			local ix=v.tx/it.tiles.bitmap_hx
			local iy=v.ty/it.tiles.bitmap_hy
			
			local ox=(v.ox)*(v.sx)
			local oy=(v.oy)*(v.sy)
			local hx=v.hx*(v.sx)
			local hy=v.hy*(v.sy)
			
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



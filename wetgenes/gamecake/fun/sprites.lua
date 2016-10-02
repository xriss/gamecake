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
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.tiles =assert(it.system.components[opts.tiles  or "tiles" ])
	it.opts=opts
	it.component="sprites"
	it.name=opts.name

	it.tile_hx=it.opts.tile_size and it.opts.tile_size[1] or it.tiles.tile_hx -- cache the tile size, or allow it to change per sprite component
	it.tile_hy=it.opts.tile_size and it.opts.tile_size[2] or it.tiles.tile_hy

	it.drawlist=opts.drawlist or { { color={1,1,1,1} , dx=0 , dy=0 } } -- use this to add drop shadows
	it.drawtype=opts.drawtype

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
		
		v.hx=v.hx or v.h or it.tile_hx
		v.hy=v.hy or v.h or it.tile_hy

		v.tx=v.tx or (           ( (v.t or 0)%256 ) )
		v.ty=v.ty or ( math.floor( (v.t or 0)/256 ) )

		v.ox=v.ox or v.hx/2
		v.oy=v.oy or v.hy/2
		
		v.px=(v.px or 0)
		v.py=(v.py or 0)

		v.rz=(v.rz or 0)

		v.sx=v.sx or v.s or 1
		v.sy=v.sy or v.s or 1

		v.zf=v.zf or 0

		v.r=v.r or ( v.color and ( v.color[1] or v.color.r ) ) or 1
		v.g=v.g or ( v.color and ( v.color[2] or v.color.g ) ) or 1
		v.b=v.b or ( v.color and ( v.color[3] or v.color.b ) ) or 1
		v.a=v.a or ( v.color and ( v.color[4] or v.color.a ) ) or 1
		
	end

	it.update=function()
	end
	
	it.draw=function()
		
		gl.Color(1,1,1,1)

		for i,dl in ipairs(it.drawlist) do

			local batch={}
			for idx,v in pairs(it.list) do

				local ixw=(v.tx*it.tile_hx+v.hx)/(it.tiles.hx)
				local iyh=(v.ty*it.tile_hy+v.hy)/(it.tiles.hy)
				local ix= (v.tx*it.tile_hx     )/(it.tiles.hx)
				local iy= (v.ty*it.tile_hy     )/(it.tiles.hy)
				
				local ox=(v.ox)*(v.sx)
				local oy=(v.oy)*(v.sy)
				local hx=v.hx*(v.sx)
				local hy=v.hy*(v.sy)
				
				local s=-math.sin(math.pi*(v.rz)/180)
				local c= math.cos(math.pi*(v.rz)/180)

				local v1=gl.apply_modelview( {dl.dx+v.px-c*(ox)-s*(oy),			dl.dy+v.py+s*(ox)-c*(oy),			v.zf,1} )
				local v2=gl.apply_modelview( {dl.dx+v.px+c*(hx-ox)-s*(oy),		dl.dy+v.py-s*(hx-ox)-c*(oy),		v.zf,1} )
				local v3=gl.apply_modelview( {dl.dx+v.px-c*(ox)+s*(hy-oy),		dl.dy+v.py+s*(ox)+c*(hy-oy),		v.zf,1} )
				local v4=gl.apply_modelview( {dl.dx+v.px+c*(hx-ox)+s*(hy-oy),	dl.dy+v.py-s*(hx-ox)+c*(hy-oy),		v.zf,1} )

				local t=
				{
					v1[1],	v1[2],	v1[3],		ix,		iy,			v.r*dl.color[1],v.g*dl.color[2],v.b*dl.color[3],v.a*dl.color[4],
					v1[1],	v1[2],	v1[3],		ix,		iy,			v.r*dl.color[1],v.g*dl.color[2],v.b*dl.color[3],v.a*dl.color[4],
					v2[1],	v2[2],	v2[3],		ixw,	iy,			v.r*dl.color[1],v.g*dl.color[2],v.b*dl.color[3],v.a*dl.color[4],
					v3[1],	v3[2],	v3[3],		ix,		iyh,		v.r*dl.color[1],v.g*dl.color[2],v.b*dl.color[3],v.a*dl.color[4],
					v4[1],	v4[2],	v4[3],		ixw,	iyh,		v.r*dl.color[1],v.g*dl.color[2],v.b*dl.color[3],v.a*dl.color[4],
					v4[1],	v4[2],	v4[3],		ixw,	iyh,		v.r*dl.color[1],v.g*dl.color[2],v.b*dl.color[3],v.a*dl.color[4],
				}
				
				local l=#batch for i,v in ipairs(t) do batch[ l+i ]=v end

			end

			flat.tristrip("rawuvrgba",batch)
			
		end

	end

	return it
end

	return sprites
end



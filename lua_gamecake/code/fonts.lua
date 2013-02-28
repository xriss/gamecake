-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local zips=require("wetgenes.zips")

local ft=require("wetgenes.freetype")
local grd=require("wetgenes.grd")
local wwin=require("wetgenes.win")

local core=require("wetgenes.gamecake.core")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,fonts)

	fonts.data={}
	
	local opts=oven.opts
	local cake=oven.cake
	local gl=oven.gl

	fonts.zip=opts.zip
	fonts.prefix=opts.fontprefix or "data/fonts/"
	fonts.postfix=opts.fontpostfix or ".ttf"


fonts.get=function(id)
	return fonts.data[id]
end

fonts.set=function(id,d)
	fonts.data[id]=d
end


--
-- unload a previously loaded image
--
fonts.unload=function(id)
	
	local t=fonts.get(id)

	if t then
		if gl then --gl mode
			for i,v in pairs(t.images or {}) do -- delete all the chars
				cake.images.unload( v.id )
			end
		end
	end
	
	fonts.set(id,nil)
end

--
-- load a single image, and make it easy to lookup by the given id
--
fonts.load=function(filename,id)
	local t=fonts.get(id)
	
	if t then return t end --first check it is not already loaded

	if type(filename)=="number" then -- builtin font id, so far we only have this one

print("Loading font 8x8")

		if gl then --gl mode
		
			t={}
			t.filename=filename
			fonts.set(id,t)
			t.size=8

			t.chars={}
			t.images={}
			
			local g=grd.create(grd.FMT_U8_RGBA_PREMULT,16*16,16*16,1)

			for i=32,127 do -- setup base textures for 7bit ascii
				
				local idx=math.floor((i-32)%16) -- 0-31 are unprintable chars
				local idy=math.floor((i-32)/16)
				
				local c={}
				t.chars[i]=c
				
				c.image=1
				
				c.add=8 -- character draw width which may be fractional

				c.tx=(idx*16)+8
				c.ty=(idy*16)+8

				c.x=-1 -- offsets to draw the bitmap at, whole pixels
				c.y=-1
				c.w=10 --size to draw, a 1 pixel border is good to have
				c.h=10
				
				c.u1=(c.tx-1)/(16*16)
				c.u2=(c.tx+9)/(16*16)
				c.v1=(c.ty-1)/(16*16)
				c.v2=(c.ty+9)/(16*16)

				g:pixels(c.tx,c.ty,8,8,wwin.glyph_8x8(i)) -- splat into grid

			end

			t.images[1]=cake.images.upload_grd(nil,g) -- send to opengl

		end
		
	else

		local fname=fonts.prefix..filename..fonts.postfix
		
		local d=assert(zips.readfile(fname))
print("Loading font ",fname,#d)		

		if gl then --gl mode
		
			t={}
			t.filename=filename
			fonts.set(id,t)
			
			t.font=ft.create()
			t.data=d -- keep the data alive (the loader expects it to continue to exist)
			t.font:load_data(t.data)
			
			
			t.size=32
			t.font:size(t.size,t.size) -- render at 32x32 pixel size, all future numbers are relative to this size
			
			t.chars={}
			t.images={}
			
			local g=grd.create(grd.FMT_U8_RGBA_PREMULT,32*16,32*16,1)
			
			local gt=grd.create() -- tempory buffer
			for i=32,127 do -- setup base textures for 7bit ascii

				local idx=math.floor((i-32)%16) -- 0-31 are unprintable chars
				local idy=math.floor((i-32)/16)

				t.font:render(i) -- render
				t.font:grd(gt) -- copy to grd
				gt:convert(grd.FMT_U8_RGBA_PREMULT)
				
--				local c=fonts.cake.images.upload_grd(nil,g) -- send to opengl
				local c={}
				t.chars[i]=c
				
				c.image=1
				
				c.x=t.font.bitmap_left -- offsets to draw the bitmap at, whole pixels
				c.y=t.size-t.font.bitmap_top
--				c.add=math.floor(t.font.advance) -- character draw width
				c.add=t.font.advance -- character draw width possibly fractional
				
				c.w=gt.width
				c.h=gt.height
				
				c.tx=(idx*32)+1
				c.ty=(idy*32)+1

				c.u1=(c.tx-1)    /(32*16)
				c.u2=(c.tx+c.w+1)/(32*16)
				c.v1=(c.ty-1)    /(32*16)
				c.v2=(c.ty+c.h+1)/(32*16)
				
				if c.w>=1 and c.h>=1 then -- must have size?
				
					local b=gt:pixels(0,0,c.w,c.h)
					g:pixels(c.tx,c.ty,c.w,c.h, b ) -- splat into grid
					
					-- remember border
					c.x=c.x-1
					c.y=c.y-1
					c.w=c.w+2
					c.h=c.h+2
					
				end

			end

	-- we keep the ttf font in memory around so we can reload chars or load new chars as we need them

			t.images[1]=cake.images.upload_grd(nil,g) -- send to opengl

			
		end
		
	end
	
	if t then
		core.fontdata_sync(t)
	end

	return t
	
end

--
-- load many images from id=filename table
--
fonts.loads=function(tab)

	for i,v in pairs(tab) do
	
		if type(i)=="number" then -- just use filename twice
			fonts.load(v,v)
		else
			fonts.load(v,i)
		end
		
		oven.preloader()
	end

end


fonts.start = function()

	for v,n in pairs(fonts.remember or {}) do
		fonts.load(v,n)
	end
	fonts.remember=nil
end

fonts.stop = function()

	fonts.remember={}
	
	for n,t in pairs(fonts.data) do

		fonts.remember[t.filename]=n		
		fonts.unload(n)

	end

end


	return fonts
end




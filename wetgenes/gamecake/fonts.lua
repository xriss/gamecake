-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.fonts")

local zips=require("wetgenes.zips")

local ft=require("wetgenes.freetype")
local grd=require("wetgenes.grd")
local wwin=require("wetgenes.win")


function bake(opts)

	local fonts={}
	
	fonts.cake=opts.cake
	fonts.gl=opts.gl
	
	fonts.data={}
	
	fonts.zip=opts.zip
	fonts.prefix=opts.fontprefix or "data/fonts/"
	fonts.postfix=opts.fontpostfix or ".ttf"


	local cake=fonts.cake
	local gl=fonts.gl


fonts.get=function(id)
	return fonts.data[id]
end

fonts.set=function(d,id)
	fonts.data[id]=d
end


--
-- unload a previously loaded image
--
fonts.unload=function(id)
	
	local t=fonts.get(id)

	if t then
		if gl then --gl mode
			for i,v in pairs(t.chars or {}) do -- delete all the chars
				gl.DeleteTexture( v.id )
			end
		end
		fonts.set(nil,id)
	end
end

--
-- load a single image, and make it easy to lookup by the given id
--
fonts.load=function(filename,id)

	local t=fonts.get(id)
	
	if t then return t end --first check it is not already loaded

	if type(filename)=="number" then -- builtin font id, so far we only have this one

		if gl then --gl mode
			t={}
			t.filename=filename
			fonts.set(t,id)
			t.size=8

			t.chars={}

			for i=32,127 do -- setup base textures for 7bit ascii
				
				local g=grd.create(grd.FMT_U8_ARGB,10,10,1) -- tempory buffer, 1 pixel border makes it less horrible when scaled
				
				local dat=wwin.glyph_8x8(i)
				g:pixels(1,1,8,8,dat)
				
				g:convert(grd.FMT_U8_RGBA_PREMULT)

				local c=fonts.cake.images.upload_grd(nil,g) -- send to opengl

				t.chars[i]=c
				
				c.x=-1 -- offsets to draw the bitmap at, whole pixels
				c.y=-1
				c.add=8 -- character draw width which may be fractional

			end

			return t
		end
		
	else

		local fname=fonts.prefix..filename..fonts.postfix
		
		
		local d=assert(zips.readfile(fname))

		if gl then --gl mode
		
			t={}
			t.filename=filename
			fonts.set(t,id)
			
			t.font=ft.create()
			t.data=d -- keep the data alive (the loader expects it to continue to exist)
			t.font:load_data(t.data)
			
			
			t.size=32
			t.font:size(t.size,t.size) -- render at 32x32 pixel size, all future numbers are relative to this size
			
			t.chars={}
			
			local g=grd.create() -- tempory buffer
			for i=32,127 do -- setup base textures for 7bit ascii

				t.font:render(i) -- render
				t.font:grd(g) -- copy to grd
				g:convert(grd.FMT_U8_RGBA_PREMULT)
				
				local c=fonts.cake.images.upload_grd(nil,g) -- send to opengl
				t.chars[i]=c
				
				c.x=t.font.bitmap_left -- offsets to draw the bitmap at, whole pixels
				c.y=t.size-t.font.bitmap_top
				c.add=math.floor(t.font.advance) -- character draw width which may be fractional

			end

	-- we keep the ttf font in memory around so we can reload chars or load new chars as we need them

			return t
			
		end
		
	end
	
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




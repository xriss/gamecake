-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.fonts")

base=require(...)
meta={}
meta.__index=base

local ft=require("wetgenes.freetype")
local grd=require("wetgenes.grd")


function bake(opts)

	local fonts={}
	setmetatable(fonts,meta)
	
	fonts.cake=opts.cake
	fonts.gl=opts.gl
	
	fonts.data={}
	
	fonts.zip=opts.zip
	fonts.prefix=opts.fontprefix or "data/font_"
	fonts.postfix=opts.fontpostfix or ".ttf"
	
	return fonts
end

get=function(fonts,id,name)
	name=name or "base"
	return fonts.data[name] and fonts.data[name][id]
end

set=function(fonts,d,id,name)
	name=name or "base"
	local tab
	
	if fonts.data[name] then
		tab=fonts.data[name]		
	else
		tab={}
		fonts.data[name]=tab
	end
	
	tab[id]=d	
end


--
-- unload a previously loaded image
--
unload=function(fonts,id,name)
	local gl=fonts.gl
	name=name or "base"
	
	local t=fonts:get(id,name)

	if t then
		if gl then --gl mode
			for i,v in pairs(t.chars or {}) do -- delete all the chars
				gl.DeleteTexture( v.id )
			end
		end
		fonts:set(nil,id,name)
	end
end

--
-- load a single image, and make it easy to lookup by the given id
--
load=function(fonts,filename,id,name)
	local gl=fonts.gl
	name=name or "base"

	local t=fonts:get(id,name)
	
	if t then return t end --first check it is not already loaded

	if type(filename)=="number" then -- builtin font id, so far we only have this one

		if gl then --gl mode
			t={}
			t.filename=filename
			fonts:set(t,id,name)
			t.size=8

			for i=32,127 do -- setup base textures for 7bit ascii

				
				local g=grd.create(grd.FMT_U8_ARGB,8,8,1) -- tempory buffer
				
				local dat="1234567890123456789012345678901234567890123456789012345678901234"
				dat=dat..dat..dat..dat -- tmp test garbage
				g:pixels(0,0,8,8,dat)
				
				g:convert(grd.FMT_U8_RGBA_PREMULT)

				local c=fonts.cake.images:upload_grd(nil,g) -- send to opengl
				t.chars[i]=c
				
				c.x=0 -- offsets to draw the bitmap at, whole pixels
				c.y=0
				c.add=8 -- character draw width which may be fractional

			end

			return t
		end
		
	else

		local fname=fonts.prefix..filename..fonts.postfix
		
	--	local g=assert(grd.create())
		
		local d
		if fonts.zip then -- load from a zip file
			local f=assert(fonts.zip:open(fname))
			d=assert(f:read("*a"))
			f:close()
		else
			local f=assert(io.open(fname,"rb"))
			d=assert(f:read("*a"))
			f:close()
		end
		
		if gl then --gl mode
		
			t={}
			t.filename=filename
			fonts:set(t,id,name)
			
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
	--			g:convert(grd.FMT_U8_ARGB_PREMULT)
				g:convert(grd.FMT_U8_RGBA_PREMULT)
				
				local c=fonts.cake.images:upload_grd(nil,g) -- send to opengl
				t.chars[i]=c
				
				c.x=t.font.bitmap_left -- offsets to draw the bitmap at, whole pixels
				c.y=t.size-t.font.bitmap_top
				c.add=t.font.advance -- character draw width which may be fractional

			end

	-- we keep the ttf font in memory around so we can reload chars or load new chars as we need them

			return t
			
		end
		
	end
	
end

--
-- load many images from id=filename table
--
loads=function(fonts,tab)

	for i,v in pairs(tab) do
	
		if type(v)=="table" then -- use a subtable and its name
		
			for ii,vv in pairs(v) do
			
				if type(ii)=="number" then -- just use filename twice
					fonts:load(i.."_"..vv,vv,i)
				else
					fonts:load(i.."_"..vv,ii,i)
				end
				
			end
			
		elseif type(i)=="number" then -- just use filename twice
			fonts:load(v,v)
		else
			fonts:load(v,i)
		end
		
	end

end


start = function(fonts)

	for v,n in pairs(fonts.remember or {}) do
		fonts:load(v,n[1],n[2])
	end
	fonts.remember=nil
end

stop = function(fonts)

	fonts.remember={}
	
	for n,tab in pairs(fonts.data) do

		for i,t in pairs(tab) do
		
			fonts.remember[t.filename]={i,n}
		
			fonts:unload(i,n)
			
		end

	end

end



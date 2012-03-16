-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.fonts")

base=require(...)
meta={}
meta.__index=base

local ft=require("wetgenes.freetype")
local grd=require("wetgenes.grd")

--[[

local ft=require("freetype")
local grd=require("wetgenes.grd")

local fp=io.open("../../../mods/data/fonts/Vera.ttf","rb")
local d=fp:read("*a")
fp:close()

--local f=ft.create("../../../mods/data/fonts/Vera.ttf")
local f=ft.create(d)

local g=grd.create()

f:size(64,64)
f:render(65)
f:grd(g[0])
g:convert(grd.FMT_U8_ARGB)
print(wstr.dump(f))
print(wstr.dump(g:info()))

]]


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
			for i,v in pairs(t.chars or {}) do -- delete all thje chars
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
		
		
		t.font:size(32,32) -- render at 32x32 pixel size
		
		t.chars={}
		
		local g=grd.create() -- tempory buffer
		for i=32,127 do -- setup base texturers

print("loading glyph "..i)

			t.font:render(i) -- render
			t.font:grd(g) -- copy to grd			
			g:convert(grd.FMT_U8_ARGB)
			
			t.chars[i]=fonts.cake.images:upload_grd(nil,g) -- send to opengl
			
		end

		return t
	else
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



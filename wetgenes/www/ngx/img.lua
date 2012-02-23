-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log
local fetch=require("wetgenes.www.any.fetch")
local wstr=require("wetgenes.string")
local grd=require("wetgenes.grd")

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.img"]=_M




function get(data,fmt)
	log("img.get:")
	
	local gfmt=grd.HINT_PNG
	if fmt=="jpeg" then gfmt=grd.HINT_JPG end
	
	local g=grd.create()
	g:load_data(data,gfmt)

--	return core.get(...)

	return g
end

function resize(g,x,y)
	log("img.resize:")

--	return core.resize(...)

	g:scale(x,y,1)

	return g
end

function composite(tab)
	log("img.composite:")

--	return core.composite(...)
--[[
					image=img.composite({
						format="DEFAULT",
						width=ix,
						height=iy,
						color=tonumber("ffffff",16), -- white, does not work?
						{image,px,py,1,"TOP_LEFT"},
					}) -- and force it to a JPEG with a white? background
]]

		return tab[0][0]
end


function memsave(g,fmt)

	local gfmt=grd.HINT_PNG
	if fmt=="jpeg" then gfmt=grd.HINT_JPG end

	local function file_read(filename)
		local fp=assert(io.open(filename,"rb"))
		local d=assert(fp:read("*a"))
		fp:close()
		return d
	end
	
	local fn=os.tmpname()
	
	g:save_file(fn,gfmt)
	
	g.body=file_read(filename)
	g.format=fmt
	
	os.remove(fn)
	
	return g
end

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local win=require("wetgenes.win")

local wgrd   =require("wetgenes.grd")
local lfs ; (function() pcall( function() lfs=require("lfs") end ) end)()

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,snaps)

	snaps=snaps or {}
	
	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local layout=cake.layouts.create{}

	function snaps.setup()
		if not lfs then return end

		lfs.mkdir(win.files_prefix.."snaps")
	end

	function snaps.clean()
	end
	
	function snaps.update()
	end
	
	function snaps.draw()
	end
		
	function snaps.msg(m)
		if not lfs then return m end

		if m.class=="key" and m.keyname=="f12" and m.action==1 then
			local name=os.date("%Y%m%d_%H%M%S")
print("Snaps "..name)
			local g=wgrd.create( wgrd.FMT_U8_RGBA_PREMULT , oven.win.width , oven.win.height , 1 )
			gl.ReadPixels(0,0,oven.win.width,oven.win.height,gl.RGBA,gl.UNSIGNED_BYTE,g.data)
			g:convert( wgrd.FMT_U8_RGBA ) -- save code expects this format
			g:flipy() -- open gl is upside down
			g:save(win.files_prefix.."snaps/"..name..".png")
			return nil
		end
		return m
	end

	return snaps
end

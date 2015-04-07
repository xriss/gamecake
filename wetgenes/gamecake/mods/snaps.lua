--
-- (C) 2013 Kriss@XIXs.com
--
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
--[[
		if snaps.auto then
			snaps.frame=snaps.frame+1
			local name=snaps.auto.."_"..(("%04d"):format(snaps.frame))
print("Snaps "..name)
			local g=wgrd.create( wgrd.FMT_U8_RGBA_PREMULT , oven.win.width , oven.win.height , 1 )
			gl.ReadPixels(0,0,oven.win.width,oven.win.height,gl.RGBA,gl.UNSIGNED_BYTE,g.data)
--			g:convert( wgrd.FMT_U8_RGBA ) -- save code expects this format
			g:flipy() -- open gl is upside down
			g:save(win.files_prefix.."snaps/"..name..".png")
			return nil
		end
]]
	end
		
	snaps.shift_key=false
	snaps.auto=false
	snaps.frame=0
	function snaps.msg(m)
		if not lfs then return m end

--print(wstr.dump(m))

--[[
		if     m.class=="key" and ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) and m.action==1 then
			snaps.shift_key=true
		elseif m.class=="key" and ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) and m.action==-1 then
			snaps.shift_key=false
		end
]]		
		if ( m.class=="key" and m.keyname=="f12" and m.action==1 ) then
--[[
			if snaps.shift_key then
				if snaps.auto then
					snaps.auto=nil
				else
					snaps.auto=os.date("%Y%m%d_%H%M%S")
					snaps.frame=0
				end
				print("auto",snaps.auto)
			end
]]
			local name=os.date("%Y%m%d_%H%M%S")
print("Snaps "..name)
			local g=wgrd.create( wgrd.FMT_U8_RGBA_PREMULT , oven.win.width , oven.win.height , 1 )
			gl.ReadPixels(0,0,oven.win.width,oven.win.height,gl.RGBA,gl.UNSIGNED_BYTE,g.data)
--			g:convert( wgrd.FMT_U8_RGBA ) -- save code expects this format
			g:flipy() -- open gl is upside down
			g:save(win.files_prefix.."snaps/"..name..".png")
			return nil
		end
		return m
	end

	return snaps
end

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,canvas)
		
-- link together sub parts
	local font={}
	local flat={}
	canvas.font,font.canvas=font,canvas
	canvas.flat,flat.canvas=flat,canvas

	local gl=oven.gl
	local cake=oven.cake
	local win=oven.win
	local fonts=cake.fonts
	local images=cake.images
	local buffers=cake.buffers

-- prefer shader that discard pixels with low alpha < 0.25
	canvas.discard_low_alpha=false

canvas.gl_default=function()

-- the default gl state, when we deviate from this we should restore it...

	gl.PixelStore(gl.PACK_ALIGNMENT,1)
	gl.PixelStore(gl.UNPACK_ALIGNMENT,1) -- the grd code expects fully packed bitmaps

	gl.state.set(gl.state_defaults)

	gl.Color(1,1,1,1)	

	repeat
		local err=gl.GetError()
		if err~=0 then
			print( "GLERROR" , gl.numtostring(err) )
		end
	until err==0
	if not canvas.NO_GL_PROGRAM_POINT_SIZE then
		pcall( function() gl.Enable(0x8642) end )-- #define GL_PROGRAM_POINT_SIZE 0x8642
		if gl.GetError() == gl.INVALID_ENUM then    -- do not repeat error
			canvas.NO_GL_PROGRAM_POINT_SIZE=true
		end
	end
		
	gl.MatrixMode(gl.MODELVIEW)

end

function canvas.delete_vbs()
	for i,v in ipairs(canvas.vbs) do
		gl.DeleteBuffer(v)
	end
	canvas.vbs={}
	canvas.vbi=1
end

function canvas.reuse_vbs()
	canvas.vbi=1
end

function canvas.get_vb()
	local vb=canvas.vbs[canvas.vbi]
	if not vb then
		vb=gl.GenBuffer()
		canvas.vbs[canvas.vbi]=vb
	end
	canvas.vbi=canvas.vbi+1
--print(canvas.vbi)
	return vb
end


function canvas.start()
end
function canvas.stop()
	canvas.delete_vbs()
end
function canvas.draw()
	if canvas.vbi_flop then
		canvas.reuse_vbs()
	end
	canvas.vbi_flop=not canvas.vbi_flop
	cake.sheets.UseSheet=nil
end

-- basic setup of canvas
	canvas.vbs={}
	canvas.vbi=1
	
	canvas.vdat_size=0
	canvas.vdat_check=function(size) -- check we have enough buffer
		if canvas.vdat_size<size then
			canvas.vdat_size=size
			canvas.vdat=pack.alloc(canvas.vdat_size) -- temp draw buffer
		end
	end
	canvas.vdat_check(1024) -- initial buffer size it may grow but this is probably more than enough
	
	canvas.flat=oven.rebake("wetgenes.gamecake.flat")
	canvas.font=oven.rebake("wetgenes.gamecake.font")
		
	return canvas
end

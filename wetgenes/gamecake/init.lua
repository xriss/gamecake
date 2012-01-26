-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- gamecake lua is a simple CPU only 2D game framework that renders to a bitmap.
-- It bears a passing resemblence to gamecake js.



local win=win

module("wetgenes.gamecake")

local wcanvas=require("wetgenes.gamecake.canvas")


base={}
meta={}
meta.__index=base

function create(opts)

	local cake={}
	setmetatable(cake,meta)
	
	opts.cake=cake -- insert cake into opts
	
	cake.grd_fmt="GRD_FMT_U8_BGRA"
	
	
	cake.canvas=wcanvas.create(opts) -- we will need a canvas to draw too, use same opts

	return cake
end


-- draw using win and opengl functions
-- do not call if you do not have fenestra and a global win setup.
base.draw = function(cake)
	local t=win.tex( cake.canvas.grd )
	t:draw()
	t:clean()
end




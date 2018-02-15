--
-- (C) 2018 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local grddiff=M

-- create a history state with the given grd as base
grdpaint.history=function(grd)

	local history={}

	history.frame=0
	history.grd=grd -- our current grd
	history.grd_old=grd -- our old grd

-- take a snapshot of this frame for latter diffing (started drawing)
	history.draw_begin_frame=function(frame)
		history.frame=frame
		history.grd_old=history.grd:clip(0,0,history.frame,history.grd.width,history.grd.height,1):duplicate()
	end

-- push the differences from frame_old to now
	history.draw_push_frame=function()
		local ga=history.grd_old
		local gb=history.grd:clip(0,0,history.frame,history.grd.width,history.grd.height,1)
	end
	
	history.undo=function() -- go back a step
	end

	history.redo=function() -- go forward a step
	end

	return history

end

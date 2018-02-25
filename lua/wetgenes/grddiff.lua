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

	history.length=0
	history.list={}
	history.frame=0
	history.grd=grd -- our current grd
	
	history.get=function(index)
		return history.list[index]
	end
	history.set=function(index,value)
		history.list[index]=value
		if index>history.length then history.length=index end
	end
	
-- take a snapshot of this frame for latter diffing (started drawing)
	history.draw_begin_frame=function(frame)
		history.frame=frame
		history.grd_diff=history.grd:clip(	0,					0,					history.frame,
											history.grd.width,	history.grd.height,	1):duplicate()
	end

-- push the differences from frame_old to now (ended drawing)
	history.draw_push_frame=function()
		local ga=history.grd_diff
		local gb=history.grd:clip(	0,					0,					history.frame,
									history.grd.width,	history.grd.height,	1)
		local it={x=0,y=0,z=0,w=history.grd.width,h=history.grd.height,d=1}
		ga:xor(gb)
		ga:shrink(it)
		if it.w>0 and it.h>0 and it.d>0 then -- some pixels have changed
			it.data=ga:pixels(it.x,it.y,it.z,it.w,it.h,it.d,"") -- get minimal xored data area as a string
		end

		it.prev=history.index -- link to prev
		it.id=history.length+1

		history.index=it.id
		history.set(it.id,it)
		if it.prev and history.get(it.prev) then -- link from prev
			history.get(it.prev).next=it.id -- link to next
		end
		
		history.grd_diff=nil -- throw away thinking grd
	end
	
	history.undo=function() -- go back a step
	end

	history.redo=function() -- go forward a step
	end

	return history

end

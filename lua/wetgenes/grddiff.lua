--
-- (C) 2018 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local cmsgpack=require("cmsgpack")

local zlib=require("zlib")
local inflate=function(d) return ((zlib.inflate())(d)) end
local deflate=function(d) return ((zlib.deflate())(d,"finish")) end

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local grddiff=M

local palette_nil=("\0\0\0\0"):rep(256) -- an empty all 0 palette

-- create a history state with the given grd as base
grddiff.history=function(grd)

	local history={}

	history.length=0
	history.list={}
	history.frame=0
	history.grd=grd -- our current grd
	
	history.get=function(index)
		if not index then return end
		local it=history.list[index]
		it=it and cmsgpack.unpack(inflate(it))
		return it
	end
	history.set=function(index,it)
		it=it and deflate(cmsgpack.pack(it))
		history.list[index]=it
		if index>history.length then history.length=index end
	end
	
-- take a snapshot of this frame for latter diffing (started drawing)
	history.draw_pull_frame=function(frame)
		history.frame=frame
		history.grd_diff=history.grd:clip(	0,					0,					history.frame,
											history.grd.width,	history.grd.height,	1):duplicate()
	end

-- push the diff from when we pulled the frame to now (ended drawing)
	history.draw_push_frame=function()
		local ga=history.grd_diff
		local gb=history.grd:clip(	0,					0,					history.frame,
									history.grd.width,	history.grd.height,	1)
		local it={x=0,y=0,z=0,w=ga.width,h=ga.height,d=1}
		ga:xor(gb)
		it.palette=ga:palette(0,256,"")
		if it.palette==palette_nil then it.palette=nil end -- no colour has changed so do not store diff
		ga:shrink(it)
		if it.w>0 and it.h>0 and it.d>0 then -- some pixels have changed
			it.data=ga:pixels(it.x,it.y,it.z,it.w,it.h,it.d,"") -- get minimal xored data area as a string
		else
			it.x=nil
			it.y=nil
			it.z=nil
			it.w=nil
			it.h=nil
			it.d=nil
		end

		it.prev=history.index -- link to prev
		it.id=history.length+1

		history.index=it.id
		history.set(it.id,it)
		if it.prev then -- link from prev
			local pit=history.get(it.prev)
			if pit then
				pit.next=it.id -- link to next
				history.set(pit.id,pit)
			end
		end
		
		history.grd_diff=nil -- throw away thinking grd
	end
	
	history.apply=function(index) -- apply diff at this index
		local it=history.get(index or history.index) -- default to current index
		if it and it.w and it.h and it.d and it.data then -- xor
			local ga=wgrd.create(history.grd.format,it.w,it.h,it.d)
			local gb=history.grd:clip(it.x,it.y,it.z,it.w,it.h,it.d)
			ga:pixels(0,0,0,it.w,it.h,it.d,it.data)
			gb:xor(ga)
		end
		if it.palette then
			local ga=wgrd.create(history.grd.format,0,0,0)
			ga:pixels(0,256,it.palette)
			history.grd:xor(ga)
		end
	end

	history.goto=function(index) -- goto this undo index
		while index>history.index do
			if not history.redo() then break end
		end
		while index<history.index do
			if not history.undo() then break end
		end
	end

	history.undo=function() -- go back a step
		local it=history.get(history.index)
		if it.prev then -- somewhere to go
			history.apply()
			history.index=it.prev
			return it
		end
	end

	history.redo=function() -- go forward a step
		local it=history.get(history.index)
		if it.next then -- somewhere to go
			history.apply(it.next)
			history.index=it.next
			return it
		end
	end

	return history

end

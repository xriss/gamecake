--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local android={}

local core=require("wetgenes.win.android.core")

local wstr=require("wetgenes.string")

--
-- simple debug print function, we wrap the core so it accepts multiple 
-- args and behaves like luas print
--
android.print=function(...)
	local t={}
	for i,v in ipairs{...} do
		t[#t+1]=tostring(v)
	end
	core.print(table.concat(t,"\t"))
end
local print=android.print



android.win_ready=false

android.create=function(opts)
print("and win prep")
	repeat
		android.queue_all_msgs()
		android.sleep(1)
	until android.win_ready
	
	return core.create(opts)
end


android.queue={}

android.queue_all_msgs=function()

	local finished=false
	repeat
	
		local ma=core.msg()
		
		if ma then
--			print("andmsg",wstr.dump(ma))
			
			if ma.cmd=="init_window" then android.win_ready=true end -- flag that it is now ok to create
			
			local m
			
			if ma.event == "app" then
			
print("andmsg",wstr.dump(ma))

				m={
					time=ma.eventtime,
					class="app",
					cmd=ma.cmd,
				}

--				if ma.cmd=="config_changed" then
--				end
			
			elseif ma.event == "motion" then
			
				local act=0
				if ma.action==0 then act= 1 end
				if ma.action==1 then act=-1 end
				if ma.action==2 then act= 0 end
				
				m={
					time=ma.eventtime,
					action=act,
					class="mouse",
					keycode=1,--ma.pointers[1].id,
					x=ma.pointers[1].x,
					y=ma.pointers[1].y,
				}

			elseif ma.event == "key" then
			
				m={
					time=ma.eventtime,
					class="key",
					ascii="",
					action=( (ma.action==0) and 1 or -1),
					keycode=ma.keycode,
					keyname=string.format("android_%02x",ma.keycode)
				}

			end
			
			if m then
--print("msg",wstr.dump(m))
				table.insert(android.queue,m)
			end
		else
			finshed=true
		end
		
	until finshed
	
end

android.msg=function()
	android.queue_all_msgs()

	if android.queue[1] then
		return table.remove(android.queue,1)
	end
	
end


android.time=core.time
android.destroy=core.destroy
android.info=core.info
android.context=core.context
android.swap=core.swap
android.sleep=core.sleep

return android

--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local android={}

local core=require("wetgenes.win.android.core")

local wstr=require("wetgenes.string")

local bit=require("bit")

local akeynames={
--start with key id of 1
"soft_left",
"soft_right",
"home",
"back",
"call",
"endcall",
"0",
"1",
"2",
"3",
"4",
"5",
"6",
"7",
"8",

"9",
"star",
"pound",
"dpad_up",
"dpad_down",
"dpad_left",
"dpad_right",
"dpad_center",
"volume_up",
"volume_down",
"power",
"camera",
"clear",
"a",
"b",
"c",

"d",
"e",
"f",
"g",
"h",
"i",
"j",
"k",
"l",
"m",
"n",
"o",
"p",
"q",
"r",
"s",

"t",
"u",
"v",
"w",
"x",
"y",
"z",
"comma",
"period",
"alt_left",
"alt_right",
"shift_left",
"shift_right",
"tab",
"space",
"sym",

"explorer",
"envelope",
"enter",
"del",
"grave",
"minus",
"equals",
"left_bracket",
"right_bracket",
"backslash",
"semicolon",
"apostrophe",
"slash",
"at",
"num",
"headsethook",

"focus",
"plus",
"menu",
"notification",
"search",
"media_play_pause",
"media_stop",
"media_next",
"media_previous",
"media_rewind",
"media_fast_forward",
"mute",
"page_up",
"page_down",
"pictsymbols",
"switch_charset",

"button_a",
"button_b",
"button_c",
"button_x",
"button_y",
"button_z",
"button_l1",
"button_r1",
"button_l2",
"button_r2",
"button_thumbl",
"button_thumbr",
"button_start",
"button_select",
"button_mode",
"escape",

"forward_del",
"ctrl_left",
"ctrl_right",
"caps_lock",
"scroll_lock",
"meta_left",
"meta_right",
"function",
"sysrq",
"break",
"move_home",
"move_end",
"insert",
"forward",
"media_play",
"media_pause",

"media_close",
"media_eject",
"media_record",
"f1",
"f2",
"f3",
"f4",
"f5",
"f6",
"f7",
"f8",
"f9",
"f10",
"f11",
"f12",
"num_lock",

"numpad_0",
"numpad_1",
"numpad_2",
"numpad_3",
"numpad_4",
"numpad_5",
"numpad_6",
"numpad_7",
"numpad_8",
"numpad_9",
"numpad_divide",
"numpad_multiply",
"numpad_subtract",
"numpad_add",
"numpad_dot",
"numpad_comma",

"numpad_enter",
"numpad_equals",
"numpad_left_paren",
"numpad_right_paren",
"volume_mute",
"info",
"channel_up",
"channel_down",
"zoom_in",
"zoom_out",
"tv",
"window",
"guide",
"dvr",
"bookmark",
"captions",

"settings",
"tv_power",
"tv_input",
"stb_power",
"stb_input",
"avr_power",
"avr_input",
"prog_red",
"prog_green",
"prog_yellow",
"prog_blue",
"app_switch",
"button_1",
"button_2",
"button_3",
"button_4",

"button_5",
"button_6",
"button_7",
"button_8",
"button_9",
"button_10",
"button_11",
"button_12",
"button_13",
"button_14",
"button_15",
"button_16",
"language_switch",
"manner_mode",
"3d_mode",
"contacts",

"calendar",
"music",
"calculator",
"zenkaku_hankaku",
"eisu",
"muhenkan",
"henkan",
"katakana_hiragana",
"yen",
"ro",
"kana",
"assist",

}


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

	repeat
		android.queue_all_msgs()
		android.sleep(1)
	until android.win_ready

	return core.create(opts)
end

android.queue={}

android.queue_all_msgs=function(win)

	local finished=false
	repeat
	
--		debug.sethook( function(event, line)
--		  local s = debug.getinfo(2).short_src
--		  print(s .. ":" .. line)
--		end, "l")

		local ma=core.msg(win)

--		debug.sethook(nil, "l")
		
		if ma then
		
--print("andmsg",wstr.dump(ma))

			
			if ma.cmd=="init_window" then android.win_ready=true end -- flag that it is now ok to create
			
			local m
			
			if ma.event == "app" then

				m={
					time=ma.eventtime,
					class="app",
					cmd=ma.cmd,
				}

--				if ma.cmd=="config_changed" then
--				end
			
			elseif ma.event == "motion" then
				
				if ma.source and bit.band( ma.source , 0x01000000 ) == 0x01000000 then -- joystick

					for i=1,#ma.pointers do
						local p=ma.pointers[i]
						table.insert(android.queue,{
							time=ma.eventtime,
							class="joystick",
							id=p.id, -- multiple joysticks I think?
							lx=p.lx,
							ly=p.ly,
							rx=p.rx,
							ry=p.ry,
							dx=p.dx,
							dy=p.dy,
							device=ma.device,
						})
					end
					
				elseif ma.action and ma.pointers then

					local act=0
					local action=ma.action%256
					local actidx=math.floor((ma.action/256)%256) + 1

					if action==0 then act= 1 end -- single touch
					if action==1 then act=-1 end

					if action==5 then act= 1 end -- multi touch
					if action==6 then act=-1 end
					
					local fingers={}
					for i=1,#ma.pointers do
						local p=ma.pointers[i]
						if act==0 or i~=actidx then -- just report position
							fingers[#fingers+1]={
								time=ma.eventtime,
								action=0,
								class="mouse",
								x=p.x,
								y=p.y,
								pressure=p.pressure,
								fingers=fingers,
								finger=p.id, -- this is a unique id for the duration of this touch
							}
						else -- this is a finger going up/down
							fingers[#fingers+1]={
								time=ma.eventtime,
								action=act,
								class="mouse",
								keycode=1,	-- always report all fingers as left mouse button
								x=p.x,
								y=p.y,
								pressure=p.pressure,
								fingers=fingers,
								finger=p.id,
							}
						end
					end
					-- send them all ourself
					for i,v in ipairs(fingers) do
						table.insert(android.queue,v)
					end
					
				end

			elseif ma.event == "key" then
			
				if bit.band( ma.source , 0x00000400 ) == 0x00000400 then -- joystick

					m={
						time=ma.eventtime,
						class="joykey",
						ascii="",
						action=( (ma.action==0) and 1 or -1),
						keycode=ma.keycode,
						keyname=akeynames[ma.keycode] or string.format("android_%02x",ma.keycode),
						device=ma.device,
					}				
					if #m.keyname==1 then m.ascii=m.keyname end -- cheap ascii hack for now
					
				else

					m={
						time=ma.eventtime,
						class="key",
						ascii="",
						action=( (ma.action==0) and 1 or -1),
						keycode=ma.keycode,
						keyname=akeynames[ma.keycode] or string.format("android_%02x",ma.keycode)
					}
					if #m.keyname==1 then m.ascii=m.keyname end -- cheap ascii hack for now

				end

			elseif ma.event == "sensor" then
					m={
						class="sensor",
						sensor=ma.sensor,
						x=ma.x,
						y=ma.y,
						z=ma.z,
						time=ma.timestamp,
					}
			end
			
			if m then
				table.insert(android.queue,m)
			end
			
--print("msg",wstr.dump(android.queue[#android.queue]))

		else
			finished=true
		end
		
	until finished
	
end

android.msg=function(win)
	android.queue_all_msgs(win)

	if android.queue[1] then
		return table.remove(android.queue,1)
	end
	
end

android.stop=function(...)
	core.stop(...)
end

android.start=function(...)
	core.start(...)
end

--
-- export all core functions not wrapped above
--
for n,v in pairs(core) do -- check the core
	if type(v)=="function" then -- only functions
		if not android[n] then -- only if not prewrapped
			android[n]=v
		end
	end
end

return android

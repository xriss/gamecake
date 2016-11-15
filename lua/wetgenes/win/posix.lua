--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local bit=require("bit")

local posix={}
local core=require("posix")

local scan={}
scan[  1] = "escape"
scan[  2] = "1"
scan[  3] = "2"
scan[  4] = "3"
scan[  5] = "4"
scan[  6] = "5"
scan[  7] = "6"
scan[  8] = "7"
scan[  9] = "8"
scan[ 10] = "9"
scan[ 11] = "0"
scan[ 12] = "minus"
scan[ 13] = "equal"
scan[ 14] = "back"
scan[ 15] = "tab"
scan[ 16] = "q"
scan[ 17] = "w"
scan[ 18] = "e"
scan[ 19] = "r"
scan[ 20] = "t"
scan[ 21] = "y"
scan[ 22] = "u"
scan[ 23] = "i"
scan[ 24] = "o"
scan[ 25] = "p"
scan[ 26] = "bracketleft"
scan[ 27] = "bracketright"
scan[ 28] = "return"
scan[ 29] = "control_l"
scan[ 30] = "a"
scan[ 31] = "s"
scan[ 32] = "d"
scan[ 33] = "f"
scan[ 34] = "g"
scan[ 35] = "h"
scan[ 36] = "j"
scan[ 37] = "k"
scan[ 38] = "l"
scan[ 39] = "semicolon"
scan[ 40] = "apostrophe"
scan[ 41] = "grave"
scan[ 42] = "shift_l"
scan[ 43] = "backslash"
scan[ 44] = "z"
scan[ 45] = "x"
scan[ 46] = "c"
scan[ 47] = "v"
scan[ 48] = "b"
scan[ 49] = "n"
scan[ 50] = "m"
scan[ 51] = "comma"
scan[ 52] = "period"
scan[ 53] = "slash"
scan[ 54] = "shift_r"
scan[ 55] = "kp_multiply"
scan[ 56] = "alt_l"
scan[ 57] = "space"
scan[ 58] = "caps_lock"
scan[ 59] = "f1"
scan[ 60] = "f2"
scan[ 61] = "f3"
scan[ 62] = "f4"
scan[ 63] = "f5"
scan[ 64] = "f6"
scan[ 65] = "f7"
scan[ 66] = "f8"
scan[ 67] = "f9"
scan[ 68] = "f10"
scan[ 69] = "num_lock"
scan[ 70] = "scroll_lock"
scan[ 71] = "kp_7"
scan[ 72] = "kp_8"
scan[ 73] = "kp_9"
scan[ 74] = "kp_subtract"
scan[ 75] = "kp_4"
scan[ 76] = "kp_5"
scan[ 77] = "kp_6"
scan[ 78] = "kp_add"
scan[ 79] = "kp_1"
scan[ 80] = "kp_2"
scan[ 81] = "kp_3"
scan[ 82] = "kp_0"
scan[ 83] = "kp_period"
scan[ 84] = "last_console"
scan[ 85] = ""
scan[ 86] = "less"
scan[ 87] = "f11"
scan[ 88] = "f12"
scan[ 89] = ""
scan[ 90] = ""
scan[ 91] = ""
scan[ 92] = ""
scan[ 93] = ""
scan[ 94] = ""
scan[ 95] = ""
scan[ 96] = "kp_enter"
scan[ 97] = "control_r"
scan[ 98] = "kp_divide"
scan[ 99] = "control_backslash"
scan[100] = "alt_r"
scan[101] = "break"
scan[102] = "find"
scan[103] = "up"
scan[104] = "prior"
scan[105] = "left"
scan[106] = "right"
scan[107] = "select"
scan[108] = "down"
scan[109] = "next"
scan[110] = "insert"
scan[111] = "delete"
scan[112] = "macro"
scan[113] = "f13"
scan[114] = "f14"
scan[115] = "help"
scan[116] = "do"
scan[117] = "f17"
scan[118] = "kp_minplus"
scan[119] = "pause"
scan[120] = ""
scan[121] = ""
scan[122] = ""
scan[123] = ""
scan[124] = ""
scan[125] = "windows_l"
scan[126] = ""
scan[127] = "windows_r"


local ascii={}
ascii["a"]={"a","A","A"}
ascii["b"]={"b","B","B"}
ascii["c"]={"c","C","C"}
ascii["d"]={"d","D","D"}
ascii["e"]={"e","E","E"}
ascii["f"]={"f","F","F"}
ascii["g"]={"g","G","G"}
ascii["h"]={"h","H","H"}
ascii["i"]={"i","I","I"}
ascii["j"]={"j","J","J"}
ascii["k"]={"k","K","K"}
ascii["l"]={"l","L","L"}
ascii["m"]={"m","M","M"}
ascii["n"]={"n","N","N"}
ascii["o"]={"o","O","O"}
ascii["p"]={"p","P","P"}
ascii["q"]={"q","Q","Q"}
ascii["r"]={"r","R","R"}
ascii["s"]={"s","S","S"}
ascii["t"]={"t","T","T"}
ascii["u"]={"u","U","U"}
ascii["v"]={"v","V","V"}
ascii["w"]={"w","W","W"}
ascii["x"]={"x","X","X"}
ascii["y"]={"y","Y","Y"}
ascii["z"]={"z","Z","Z"}

ascii["grave"]			={"`","~"}
ascii["1"]				={"1","!"}
ascii["2"]				={"2","@"}
ascii["3"]				={"3","#"}
ascii["4"]				={"4","$"}
ascii["5"]				={"5","%"}
ascii["6"]				={"6","^"}
ascii["7"]				={"7","&"}
ascii["8"]				={"8","*"}
ascii["9"]				={"9","("}
ascii["0"]				={"0",")"}
ascii["minus"]			={"-","_"}
ascii["equal"]			={"=","+"}

ascii["tab"]			={"\t","\t"}
ascii["bracketleft"]	={"[","{"}
ascii["bracketright"]	={"]","}"}

ascii["semicolon"]		={";",":"}
ascii["apostrophe"]		={"'","\""}
ascii["backslash"]		={"\\","|"}

ascii["less"]			={"<",">"}
ascii["space"]			={" "," "}
ascii["comma"]			={",","<"}
ascii["period"]			={".",">"}
ascii["slash"]			={"/","?"}



posix.win_mouse={x=0,y=0}
posix.win_caps=false
posix.win_shift=false


function posix.win_translate_msg_keys_and_mouse(win,m)
	if not m then return nil end
	
	if m.class=="posix_mouse" then

		local function adjust(dx,dy)

			local inp=posix.win_mouse
			
			inp.x=inp.x+dx
			inp.y=inp.y+dy
			

			if inp.x<0   then inp.x=0    end
			if inp.y<0   then inp.y=0    end
			if inp.x>=win.width  then inp.x=win.width-1 end
			if inp.y>=win.height then inp.y=win.height-1 end

			table.insert(posix.win_msgstack,
			{time=os.time(),class="mouse",x=inp.x,y=inp.y,action=0,keycode=0})
			
		end
		local function click(code,act)

			local inp=posix.win_mouse

			table.insert(posix.win_msgstack,
			{time=os.time(),class="mouse",x=inp.x,y=inp.y,action=act,keycode=code})

		end

	--	print(m.class,m.type,m.code,m.value)

		if m.type==2 then -- movement
		
			local v=m.value
			if v >= 0x80000000 then v=v-0x100000000 end -- maybe 32bit
			if v >= 0x8000 then v=v-0x10000 end         -- maybe 16bit

			if m.code==0 then -- x
				adjust(v,0)
			elseif m.code==1 then -- y
				adjust(0,v)			
			elseif m.code==8 then -- mouse wheel
				if v>0 then
					for i=1,v do click(4,1) click(4,-1) end -- fake buttons
				elseif v<0 then
					for i=-1,v,-1 do click(5,1) click(5,-1) end -- fake buttons
				end
	--			print("z",v)
			end
			
		elseif m.type==1 then -- buttons
		
			local k=1+m.code-272 -- 1 is the left mouse button
			
			if k==2 then k=3 elseif k==3 then k=2 end -- middle / right need numbers swapped
			
			if m.value==0 then --up
				click(k,-1)
			elseif m.value==1 then --down
				click(k, 1)		
			end
		
		end
		
		return nil
	end

	if m.class=="posix_keyboard" then -- re jiggle to a normal key msg

		if m.type==17 and m.code==1 and m.value==1 then posix.win_caps=false end -- reset caps
		if m.type~=1 then return nil end -- ignore

		local name=scan[m.code] or ""

--print("pkey",m.type,m.code,m.value,v)

		local act=0
		if m.value==0 then act=-1 end
		if m.value==1 then act= 1 end

		if name=="caps_lock" and act==-1 then -- caps toggle
			posix.win_caps=not posix.win_caps
		end
		if name=="shift_l" or name=="shift_r" then -- shift
			if act==1 then
				posix.win_shift=true
			elseif act==-1 then
				posix.win_shift=false
			end
		end
		
		local asc=ascii[name]
		if asc then -- handle caps and shift lookup
			local ai=1
			if posix.win_caps and asc[3] then ai=3 end -- caps
			if posix.win_shift then -- shift
				if ai==1 and asc[2] then ai=2 else ai=1 end
			end
			asc=asc[ai] -- finaly pick the ascii
		end

--print("key",act,v,asc or "",name or "unknown")

		table.insert(posix.win_msgstack,
		{time=os.time(),class="key",action=act,keycode=v,ascii=asc or "",keyname=name or "unknown"})

		return nil
	end
	
	return m
end

--
-- to enable posix translations do the following (needed for raspi commandline code)
--
--posix.win_translate_msg=posix.win_translate_msg_keys_and_mouse

function posix.win_open_events(w)

	posix.win_msgstack={}
	posix.win_events={}

	local events=posix.win_events
	local fp=io.open("/proc/bus/input/devices","r")
	local tab={}
	if fp then
		for l in fp:lines() do
			local t=l:sub(1,3)
			local v=l:sub(4)
			if t=="I: " then
				tab={} -- start new device
				tab.calibration={}
				tab.bus		=string.match(v,"Bus=([^%s]+)")
				tab.vendor	=string.match(v,"Vendor=([^%s]+)")
				tab.product	=string.match(v,"Product=([^%s]+)")
				tab.version	=string.match(v,"Version=([^%s]+)")
			end
			if t=="N: " then
				tab.name=string.match(v,"Name=\"([^\"]+)")
			end
			if t=="H: " then
				local t=string.match(v,"Handlers=(.+)")
				tab.event=tonumber(string.match(t,"event(%d+)"))
				tab.js=tonumber(string.match(t,"js(%d+)"))
				tab.mouse=tonumber(string.match(t,"mouse(%d+)"))
				if tab.event and events[tab.event] then
					events[tab.event]=tab
					tab.handlers={}
					for n in string.gmatch(t,"[^%s]+") do
						tab.handlers[n]=true
						if n:sub(1,5)~="event" then -- we already have events
							events[n]=tab
						end
					end
				end
			end
		end
		fp:close()
	end
--	print(wstr.dump(events))

	local kbdcount=0
	for i=0,#events do local v=events[i]
		if v then
			v.state=v.state or {}
			if v.handlers.kbd then -- open as keyboard, there may be many of these and it is all a hacky
				v.fd=posix.open("/dev/input/event"..v.event, bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) )
				if v.fd then
					print("opened keyboard "..kbdcount.." on event"..v.event.." "..v.name)
					v.fd_device=kbdcount
					v.fd_type="keyboard"
					kbdcount=kbdcount+1
				else
--					print("failed to open keyboard "..kbdcount.." on event"..v.event.." "..v.name)
				end
			elseif v.js then -- open as joystick	
				v.fd=posix.open("/dev/input/event"..v.event, bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) )
				if v.fd then
					print("opened joystick "..v.js.." on event"..v.event.." "..v.name)
					v.fd_device=v.js
					v.fd_type="joystick"
				else
--					print("failed to open joystick "..v.js.." on event"..v.event.." "..v.name)
				end
			elseif v.mouse then -- open as mouse
				v.fd=posix.open("/dev/input/event"..v.event, bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) )
				if v.fd then
					print("opened mouse "..v.mouse.." on event"..v.event.." "..v.name)
					v.fd_device=v.mouse
					v.fd_type="mouse"
				else
--					print("failed to open mouse "..v.mouse.." on event"..v.event.." "..v.name)
				end
			end
		end
	end

--	print(wstr.dump(events))

	
end
function posix.win_read_events(w) -- call this until it returns nil to get all events
	
	local deadzone=48 -- a big one helps deal with loose sticks

	local events=posix.win_events
	for i=0,#events do local v=events[i]
		if v then
			if v.fd then
--print("read",v.name)
				local active=true
				while active do
					local pkt=posix.read(v.fd,24)
					if pkt then

						local Isecs,Imicros,Itype,Icode,Ivalue
						if #pkt==24 then -- 64bit packet hax?
							Isecs=pack.read(pkt,"u32",0)
							Imicros=pack.read(pkt,"u32",8)
							Itype=pack.read(pkt,"u16",16)
							Icode=pack.read(pkt,"u16",18)
							Ivalue=pack.read(pkt,"u16",20)
						elseif #pkt==16 then
							Isecs=pack.read(pkt,"u32",0)
							Imicros=pack.read(pkt,"u32",4)
							Itype=pack.read(pkt,"u16",8)
							Icode=pack.read(pkt,"u16",10)
							Ivalue=pack.read(pkt,"u16",12)
						end
	--print("got",v.name)
						if Isecs then -- sanity check
						
							if Ivalue>=32768 then	-- assume signed 16bit (the ps3 is unsigned 8bit)
								Ivalue=Ivalue-65536
							end
							
							if Itype==3 then -- auto calibrate on use...
								local cc=v.calibration[Icode]
								if not cc then -- new
									cc={}
									v.calibration[Icode]=cc
									cc.min=-255
									cc.max=255
								end
								if Ivalue<cc.min then cc.min=Ivalue end -- xbox may be +- 32768
								if Ivalue>cc.max then cc.max=Ivalue end
								
--								if cc.max>cc.min then
									if Ivalue>=0 then
										Ivalue=Ivalue/cc.max
									elseif Ivalue<=0 then
										Ivalue=Ivalue/(-cc.min)
									end
--									Ivalue=(Ivalue-cc.min)/(cc.max-cc.min)
--								else
--									Ivalue=0
--								end
							end

--[[
							if Itype==3 then
								if Ivalue>128-deadzone and Ivalue<128+deadzone then Ivalue=128 end
								if Ivalue<=128-deadzone then Ivalue=math.floor(128       *((    Ivalue /(128-deadzone)))) end
								if Ivalue>=128+deadzone then Ivalue=math.floor(256 - (128*((256-Ivalue)/(128-deadzone)))) end
							end
]]
							
							local key=Itype..":"..Icode
							local val=v.state[key]
							if val and (val ~= Ivalue) then val=false end

							if not val then -- report *new* values only, ignore most junk packets
								v.state[key]=Ivalue
								
								local tab={}
								
								tab.type=Itype
								tab.code=Icode
								tab.value=Ivalue							
								tab.time=Isecs+(Imicros/1000000)
								
								tab.class="posix_"..v.fd_type
								tab.posix_num=v.fd_device
								tab.posix_name=v.name
								
	--							tab.posix_device=v -- please do not edit this

								if posix.win_translate_msg then -- possibly convert to another msg?
									tab=posix.win_translate_msg(tab)
								end
								if tab then -- may have been dealt with above
									table.insert(posix.win_msgstack,tab)
								end
							end
						end
					else
						active=false
					end
				end
			end
		end
	end
	
	return table.remove(posix.win_msgstack,1) -- we pushed msgs above, pop and return them here
end

function posix.win_close_events(w)
	posix.win_events=nil
	posix.win_msgstack=nil
end

function posix.win_msg(w)
	if not posix.win_events then -- need to initialize
		posix.win_open_events(w)
	end
	return posix.win_read_events(w)
end



--
-- export all core functions not wrapped above
--
for n,v in pairs(core) do -- export the core
		if not posix[n] then -- only if not prewrapped
			posix[n]=v
		end
end


return posix

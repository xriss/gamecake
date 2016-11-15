--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local nacl={}

local core=require("wetgenes.win.nacl.core")

local wstr=require("wetgenes.string")


local import=[[

#define VK_LBUTTON	1
#define VK_RBUTTON	2
#define VK_CANCEL	3
#define VK_MBUTTON	4
#if (_WIN32_WINNT >= 0x0500)
#define VK_XBUTTON1	5
#define VK_XBUTTON2	6
#endif
#define VK_BACK	8
#define VK_TAB	9
#define VK_CLEAR	12
#define VK_RETURN	13
#define VK_SHIFT	16
#define VK_CONTROL	17
#define VK_MENU	18
#define VK_PAUSE	19
#define VK_CAPITAL	20
#define VK_KANA	0x15
#define VK_HANGEUL	0x15
#define VK_HANGUL	0x15
#define VK_JUNJA	0x17
#define VK_FINAL	0x18
#define VK_HANJA	0x19
#define VK_KANJI	0x19
#define VK_ESCAPE	0x1B
#define VK_CONVERT	0x1C
#define VK_NONCONVERT	0x1D
#define VK_ACCEPT	0x1E
#define VK_MODECHANGE	0x1F
#define VK_SPACE	32
#define VK_PRIOR	33
#define VK_NEXT	34
#define VK_END	35
#define VK_HOME	36
#define VK_LEFT	37
#define VK_UP	38
#define VK_RIGHT	39
#define VK_DOWN	40
#define VK_SELECT	41
#define VK_PRINT	42
#define VK_EXECUTE	43
#define VK_SNAPSHOT	44
#define VK_INSERT	45
#define VK_DELETE	46
#define VK_HELP	47
#define VK_LWIN	0x5B
#define VK_RWIN	0x5C
#define VK_APPS	0x5D
#define VK_SLEEP	0x5F
#define VK_NUMPAD0	0x60
#define VK_NUMPAD1	0x61
#define VK_NUMPAD2	0x62
#define VK_NUMPAD3	0x63
#define VK_NUMPAD4	0x64
#define VK_NUMPAD5	0x65
#define VK_NUMPAD6	0x66
#define VK_NUMPAD7	0x67
#define VK_NUMPAD8	0x68
#define VK_NUMPAD9	0x69
#define VK_MULTIPLY	0x6A
#define VK_ADD	0x6B
#define VK_SEPARATOR	0x6C
#define VK_SUBTRACT	0x6D
#define VK_DECIMAL	0x6E
#define VK_DIVIDE	0x6F
#define VK_F1	0x70
#define VK_F2	0x71
#define VK_F3	0x72
#define VK_F4	0x73
#define VK_F5	0x74
#define VK_F6	0x75
#define VK_F7	0x76
#define VK_F8	0x77
#define VK_F9	0x78
#define VK_F10	0x79
#define VK_F11	0x7A
#define VK_F12	0x7B
#define VK_F13	0x7C
#define VK_F14	0x7D
#define VK_F15	0x7E
#define VK_F16	0x7F
#define VK_F17	0x80
#define VK_F18	0x81
#define VK_F19	0x82
#define VK_F20	0x83
#define VK_F21	0x84
#define VK_F22	0x85
#define VK_F23	0x86
#define VK_F24	0x87
#define VK_NUMLOCK	0x90
#define VK_SCROLL	0x91
#define VK_LSHIFT	0xA0
#define VK_RSHIFT	0xA1
#define VK_LCONTROL	0xA2
#define VK_RCONTROL	0xA3
#define VK_LMENU	0xA4
#define VK_RMENU	0xA5
#if (_WIN32_WINNT >= 0x0500)
#define VK_BROWSER_BACK	0xA6
#define VK_BROWSER_FORWARD	0xA7
#define VK_BROWSER_REFRESH	0xA8
#define VK_BROWSER_STOP	0xA9
#define VK_BROWSER_SEARCH	0xAA
#define VK_BROWSER_FAVORITES	0xAB
#define VK_BROWSER_HOME	0xAC
#define VK_VOLUME_MUTE	0xAD
#define VK_VOLUME_DOWN	0xAE
#define VK_VOLUME_UP	0xAF
#define VK_MEDIA_NEXT_TRACK	0xB0
#define VK_MEDIA_PREV_TRACK	0xB1
#define VK_MEDIA_STOP	0xB2
#define VK_MEDIA_PLAY_PAUSE	0xB3
#define VK_LAUNCH_MAIL	0xB4
#define VK_LAUNCH_MEDIA_SELECT	0xB5
#define VK_LAUNCH_APP1	0xB6
#define VK_LAUNCH_APP2	0xB7
#endif
#define VK_OEM_1	0xBA
#if (_WIN32_WINNT >= 0x0500)
#define VK_OEM_PLUS	0xBB
#define VK_OEM_COMMA	0xBC
#define VK_OEM_MINUS	0xBD
#define VK_OEM_PERIOD	0xBE
#endif
#define VK_OEM_2	0xBF
#define VK_OEM_3	0xC0
#define VK_OEM_4	0xDB
#define VK_OEM_5	0xDC
#define VK_OEM_6	0xDD
#define VK_OEM_7	0xDE
#define VK_OEM_8	0xDF
#if (_WIN32_WINNT >= 0x0500)
#define VK_OEM_102	0xE2
#endif
#define VK_PROCESSKEY	0xE5
#if (_WIN32_WINNT >= 0x0500)
#define VK_PACKET	0xE7
#endif
#define VK_ATTN	0xF6
#define VK_CRSEL	0xF7
#define VK_EXSEL	0xF8
#define VK_EREOF	0xF9
#define VK_PLAY	0xFA
#define VK_ZOOM	0xFB
#define VK_NONAME	0xFC
#define VK_PA1	0xFD
#define VK_OEM_CLEAR	0xFE

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39

#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

]]

vkeys={}

for l in import:gmatch("([^\n]*)") do
	local define,value
	local state="start"
	for w in l:gmatch("([^%s]+)") do
		if state=="start" then
			if w=="#define" then
				state="define"
			else
				break
			end
		elseif state=="define" then
			define=w
			state="value"
		elseif state=="value" then
			value=w
				if define:sub(1,3)=="VK_" then -- sanity check
					define=define:sub(4)
					
					if value:sub(1,3)=="VK_" then -- allow lookback
						value=vkeys[value:sub(4)]
					end
					
					vkeys[define]=tonumber(value)
				end
			break
		else
			break
		end
	end
end
nkeys={}
for v,n in pairs(vkeys) do
	nkeys[n]=v
end

import=nil -- free it just because





--
-- simple debug print function, we wrap the core so it accepts multiple 
-- args and behaves like luas print
--
nacl.print=function(...)
	local t={}
	for i,v in ipairs{...} do
		t[#t+1]=tostring(v)
	end
	core.print(table.concat(t,"\t"))
end
local print=nacl.print


nacl.swap_pending=false
function nacl.swap()
	nacl.swap_pending=true
--print("swap start",nacl.time())
	core.swap(function()
--print("swap stop",nacl.time())
		nacl.swap_pending=false
	end)
end
	
nacl.context=core.context
nacl.time=core.time

nacl.call=core.call
nacl.getURL=core.getURL

nacl.info=core.info

nacl.queue={}

local mousenames={"left","middle","right","wheel_add","wheel_sub"}
local lastm
nacl.input_event=function(...)
--	print("NACL",...)
	local a={...}
	local m
	
	if a[1]=="mouse" then
	
		local act=0
		if a[2]==0 then act=1 end -- down
		if a[2]==1 then act=-1 end --up
		
		local k=a[3]+1

		m={
			time=nacl.time(),
			action=act,
			class="mouse",
			keycode=k,--ma.pointers[1].id,
			keyname=mousenames[k],
			x=a[4],
			y=a[5],
		}
		
	elseif a[1]=="wheel" then
	
		local k=0
		
		if a[4]>0 then
			k=4
		elseif a[4]<0 then
			k=5
		end

		m={
			time=nacl.time(),
			action=-1,
			class="mouse",
			keycode=k,
			keyname=mousenames[k],
		}
		
	elseif a[1]=="key" then
	
		if a[2]==7 then -- down
			m={
				time=nacl.time(),
				class="key",
				action=1,
				ascii="",
				keycode=a[3],
				keyname= nkeys[ a[3] ] or string.format("nacl_%02x",a[3]),			
			}
			lastm=m
		elseif a[2]==8 then -- up
			m={
				time=nacl.time(),
				class="key",
				action=-1,
				ascii="",
				keycode=a[3],
				keyname= nkeys[ a[3] ] or string.format("nacl_%02x",a[3]),			
			}
			
			if lastm then m.ascii=lastm.ascii end -- hax
			
		elseif a[2]==9 then -- ascii
		
			if lastm then lastm.ascii=a[4] end -- hax
			
		end
	
	end
	
	if m then
		table.insert(nacl.queue,m)
	end
end

nacl.msg=function()
	if nacl.queue[1] then
		return table.remove(nacl.queue,1)
	end
end


-- send message to js (nacl only)
nacl.js_post=function(m,d)
	if type(m)=="table" then
		local s={}
		for n,v in pairs(m) do
			if type(n)=="string" then
				table.insert(n.."="..wstr.url_encode(v)) -- need to escape values...
			end
		end
		s=table.concat(s,"&").."\n"
		if m[0] then s=s..m[0] end -- and we have a large data chunk after message
		return core.js_post(s)
	end
	if d then return core.js_post(m..d) end
	return core.js_post(m)
end

--
-- export all core functions not wrapped above
--
for n,v in pairs(core) do -- check the core
	if type(v)=="function" then -- only functions
		if not nacl[n] then -- only if not prewrapped
			nacl[n]=v
		end
	end
end


return nacl

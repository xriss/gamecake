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


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


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

#define VK_ENTER	10

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

local vkeys={}
local VK=vkeys

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
local nkeys={}
for v,n in pairs(vkeys) do
	nkeys[n]=v
end

import=nil -- free it just because

-- basic american keyboard mappings
local map_vkeys={

[0x29]=VK.OEM_3, -- tilde
[0x02]=VK["1"],
[0x03]=VK["2"],
[0x04]=VK["3"],
[0x05]=VK["4"],
[0x06]=VK["5"],
[0x07]=VK["6"],
[0x08]=VK["7"],
[0x09]=VK["8"],
[0x0a]=VK["9"],
[0x0b]=VK["0"],
[0x0c]=VK.OEM_MINUS, -- -
[0x0d]=VK.OEM_PLUS,  -- =
[0x10]=VK.Q,
[0x11]=VK.W,
[0x12]=VK.E,
[0x13]=VK.R,
[0x14]=VK.T,
[0x15]=VK.Y,
[0x16]=VK.U,
[0x17]=VK.I,
[0x18]=VK.O,
[0x19]=VK.P,
[0x1a]=VK.OEM_4,
[0x1b]=VK.OEM_6,
[0x2b]=VK.OEM_5,
[0x1e]=VK.A,
[0x1f]=VK.S,
[0x20]=VK.D,
[0x21]=VK.F,
[0x22]=VK.G,
[0x23]=VK.H,
[0x24]=VK.J,
[0x25]=VK.K,
[0x26]=VK.L,
[0x27]=VK.OEM_1,
[0x28]=VK.OEM_7,
[0x2c]=VK.Z,
[0x2d]=VK.X,
[0x2e]=VK.C,
[0x2f]=VK.V,
[0x30]=VK.B,
[0x31]=VK.N,
[0x32]=VK.M,
[0x33]=VK.OEM_COMMA,
[0x34]=VK.OEM_PERIOD,
[0x35]=VK.OEM_2,
[0x56]=VK.OEM_102,
[0x53]=VK.DECIMAL,

[0x01]=VK.ESCAPE,
[0x0e]=VK.BACK,
[0x0f]=VK.TAB,
[0x1c]=VK.ENTER,
[0x1d]=VK.LCTRL,
[0x2a]=VK.LSHIFT,
[0x36]=VK.RSHIFT,
[0x37]=VK.MULTIPLY,
[0x38]=VK.LALT,
[0x39]=VK.SPACE,
[0x3a]=VK.CAPITAL,
[0x3b]=VK.F1,
[0x3c]=VK.F2,
[0x3d]=VK.F3,
[0x3e]=VK.F4,
[0x3f]=VK.F5,
[0x40]=VK.F6,
[0x41]=VK.F7,
[0x42]=VK.F8,
[0x43]=VK.F9,
[0x44]=VK.F10,
[0x45]=VK.PAUSE,
[0x46]=VK.SCROLL,
[0x47]=VK.NUMPAD7,
[0x48]=VK.NUMPAD8,
[0x49]=VK.NUMPAD9,
[0x4a]=VK.SUBTRACT,
[0x4b]=VK.NUMPAD4,
[0x4c]=VK.NUMPAD5,
[0x4d]=VK.NUMPAD6,
[0x4e]=VK.ADD,
[0x4f]=VK.NUMPAD1,
[0x50]=VK.NUMPAD2,
[0x51]=VK.NUMPAD3,
[0x52]=VK.NUMPAD0,
[0x53]=VK.DELETE,
--[0x54]=VK.Sys Req,
[0x57]=VK.F11,
[0x58]=VK.F12,
[0x61]=VK.RCTRL,
[0x64]=VK.RALT,
[0x7c]=VK.F13,
[0x7d]=VK.F14,
[0x7e]=VK.F15,
[0x7f]=VK.F16,
[0x80]=VK.F17,
[0x81]=VK.F18,
[0x82]=VK.F19,
[0x83]=VK.F20,
[0x84]=VK.F21,
[0x85]=VK.F22,
[0x86]=VK.F23,
[0x87]=VK.F24,

[28]=VK.RETURN,
[96]=VK.ENTER,
[111]=VK.DELETE,
[103]=VK.UP,
[108]=VK.DOWN,
[105]=VK.LEFT,
[106]=VK.RIGHT,


--125 left windows
--127 right menu

}

local map_ascii={

[VK.A]={"a","A","A"},
[VK.B]={"b","B","B"},
[VK.C]={"c","C","C"},
[VK.D]={"d","D","D"},
[VK.E]={"e","E","E"},
[VK.F]={"f","F","F"},
[VK.G]={"g","G","G"},
[VK.H]={"h","H","H"},
[VK.I]={"i","I","I"},
[VK.J]={"j","J","J"},
[VK.K]={"k","K","K"},
[VK.L]={"l","L","L"},
[VK.M]={"m","M","M"},
[VK.N]={"n","N","N"},
[VK.O]={"o","O","O"},
[VK.P]={"p","P","P"},
[VK.Q]={"q","Q","Q"},
[VK.R]={"r","R","R"},
[VK.S]={"s","S","S"},
[VK.T]={"t","T","T"},
[VK.U]={"u","U","U"},
[VK.V]={"v","V","V"},
[VK.W]={"w","W","W"},
[VK.X]={"x","X","X"},
[VK.Y]={"y","Y","Y"},
[VK.Z]={"z","Z","Z"},

[ VK["1"] ]={"1","!"},
[ VK["2"] ]={"2","@"},
[ VK["3"] ]={"3","#"},
[ VK["4"] ]={"4","$"},
[ VK["5"] ]={"5","%"},
[ VK["6"] ]={"6","^"},
[ VK["7"] ]={"7","&"},
[ VK["8"] ]={"8","*"},
[ VK["9"] ]={"9","("},
[ VK["0"] ]={"0",")"},

[VK.OEM_MINUS]={"-","_"},
[VK.OEM_PLUS]={"=","+"},

[VK.TAB]={"\t","\t"},

[VK.OEM_4]={"[","{"},
[VK.OEM_6]={"]","}"},

[VK.OEM_1]={";",":"},
[VK.OEM_7]={"'","\""},
[VK.OEM_5]={"\\","|"},

[VK.OEM_102]={"<",">"},

[VK.OEM_COMMA]={",","<"},
[VK.OEM_PERIOD]={".",">"},
[VK.OEM_2]={"/","?"},

[VK.SPACE]={" "," "},

}



function M.bake(oven,keys)

	keys=keys or {}

	local win=oven.win
	local cake=oven.cake
	local canvas=cake.canvas
	local layout=oven.rebake("wetgenes.gamecake.mods.layout").keys

	if win.flavour=="raspi" then
		keys.posix=true -- auto translate posix msgs
	end

	keys.caps=false
	keys.shift=false

-- push a keyboard widget into a master
	function keys.setup_keyboard_widgets(master)
		local shift=false
		local top
		local hooks={}
		function hooks.click(widget)
--	print(widget.id)
			local ascii=widget.text
			local code=0
			local name=""
			
			if ascii=="<" then
				ascii=""
				code=0
				name="back"
			elseif ascii==">" then
				ascii=""
				code=0
				name="delete"
			elseif ascii=="^" then -- toggle caps
			
				shift=not shift
			
				for i,t in ipairs(top) do -- each row of keys
					for i,v in ipairs(t) do -- each key
					
						local otext=v.text
						local ntext
						if otext then
							if shift then
								ntext=string.upper(otext)
							else
								ntext=string.lower(otext)
							end
							if otext~=ntext then
								v.text=ntext
								v:set_dirty()
							end
						end
					end
				end
				
				return

			end
			
			local mstack=oven.win.msgstack
			mstack[#mstack+1]={
				time=os.time(),
				class="key",
				action=1,
				ascii=ascii,
				keycode=code,
				keyname=name,
				softkey=true,
			}
			mstack[#mstack+1]={
				time=os.time(),
				class="key",
				action=-1,
				ascii=ascii,
				keycode=code,
				keyname=name,
				softkey=true,
			}

		end

		master:clean_all()
		master.ids={}

		top=master:add({hx=320,hy=160,class="fill",fbo=true})

		local function key_line(ks)
			local t=top:add({hx=320,hy=32,class="fill"})
			for i=1,#ks do
				local k=ks:sub(i,i)
				t:add({hx=320/10,hy=32,color=0xffcccccc,text=k,id="key",hooks=hooks})
			end
		end
		
		key_line("1234567890")
		key_line("qwertyuiop")
		key_line("asdfghjkl ")
		key_line("^zxcvbnm:#")
		key_line("<    _,./>")
					
	end

	function keys.setup()
	

		keys.master=oven.rebake("wetgenes.gamecake.widgets").setup({font="Vera",text_size=24})
		
		keys.setup_keyboard_widgets(keys.master)
	
		keys.master:layout()

	end

	function keys.clean()
	
	end
	
	

	function keys.update()
	
		if layout.active then


			if keys.top.hx~=layout.w or keys.top.hy~=layout.h then -- change rez
				keys.top.hx=layout.w
				keys.top.hy=layout.h
				
				keys.master.text_size=math.floor(layout.h/6)
				keys.master:layout()
			end

			keys.master:update()
		
		end
		
	end
	
	function keys.draw()
	
		local win=oven.win
		local cake=oven.cake
		local gl=cake.gl
		local font=canvas.font

		if layout.active then

		layout.apply()
--		layout.viewport() -- did our window change?
--		layout.project23d(layout.w,layout.h,1/4,layout.h*4)
		canvas.gl_default() -- reset gl state

--		gl.MatrixMode(gl.PROJECTION)
--		gl.LoadMatrix( layout.pmtx )

--		gl.MatrixMode(gl.MODELVIEW)
--		gl.LoadIdentity()
--		gl.Translate(-layout.w/2,-layout.h/2,-layout.h*2) -- top left corner is origin
		gl.PushMatrix()


		keys.master:draw()
			
		gl.PopMatrix()

		end

		
	end
		
	function keys.msg(m)
	
		if keys.posix then -- posix translation enabled
			if m.class=="posix_keyboard" then -- re jiggle to a normal key msg
			
				if m.type==17 and m.code==1 and m.value==1 then keys.caps=false end -- reset caps
				if m.type~=1 then return nil end -- ignore

				local v=map_vkeys[m.code] or 0

--print("pkey",m.type,m.code,m.value,v)

				local act=0
				if m.value==0 then act=-1 end
				if m.value==1 then act= 1 end

				if v==VK.CAPITAL and act==-1 then -- caps toggle
					keys.caps=not keys.caps
				end
				if v==VK.LSHIFT or v==VK.RSHIFT then -- shift
					if act==1 then
						keys.shift=true
					elseif act==-1 then
						keys.shift=false
					end
				end
				
				local n=nkeys[v]
				local asc=map_ascii[v or 0]
				
				if asc then -- handle caps and shift lookup
					local ai=1
					if keys.caps and asc[3] then ai=3 end -- caps
					if keys.shift then -- shift
						if ai==1 and asc[2] then ai=2 else ai=1 end
					end
					asc=asc[ai] -- finaly pick the ascii
				end

				win:push_msg{time=os.time(),class="key",action=act,keycode=v,ascii=asc or "",keyname=n or "unknown"}
			
				return nil -- delete msg
			end
		end
	
		if layout.active then
			if m.xraw and m.yraw then	-- we need to fix raw x,y numbers
				m.x,m.y=canvas.xyscale(m.xraw,m.yraw)	-- local coords, 0,0 is center of screen
				m.x=m.x+(layout.w/2)
				m.y=m.y+(layout.h/2)
			end
			keys.master:msg(m)
		end
		return m
		
	end
	
	return keys
end

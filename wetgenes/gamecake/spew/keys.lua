--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,keys)

	keys=keys or {}
	
	local cake=oven.cake
	local canvas=cake.canvas
	
	local mkeys=oven.rebake("wetgenes.gamecake.mods.keys")
	local recaps=oven.rebake("wetgenes.gamecake.spew.recaps")
	
	keys.defaults={}
-- single player covering entire keyboard
	keys.defaults[0]={
		["w"]			=	"up",
		["s"]			=	"down",
		["a"]			=	"left",
		["d"]			=	"right",
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["tab"]			=	"select",
		["enter"]		=	"start",
		["lshift"]		=	{"fire","x"},
		["<"]			=	{"fire","y"},
		["z"]			=	{"fire","y"},
		["."]			=	{"fire","x"},
		["/"]			=	{"fire","x"},
		["rshift"]		=	{"fire","y"},
		["lcontrol"]	=	{"fire","a"},
		["lmenu"]		=	{"fire","b"},
		["space"]		=	"fire",
		["rmenu"]		=	{"fire","x"},
		["rcontrol"]	=	{"fire","y"},
		["-"]			=	"l1",
		["["]			=	"l2",
		["="]			=	"r1",
		["]"]			=	"r2",
	}
-- 1up/2up key islands
	keys.defaults[1]={
		["w"]			=	"up",
		["s"]			=	"down",
		["a"]			=	"left",
		["d"]			=	"right",
		["lshift"]		=	"fire",
		["lcontrol"]	=	"fire",
		["lmenu"]		=	"fire",
	}
	keys.defaults[2]={
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["rshift"]		=	"fire",
		["rcontrol"]	=	"fire",
		["rmenu"]		=	"fire",
	}
-- single player mame/picade style buttons
	keys.defaults["pimoroni"]={
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["5"]			=	"select",
		["enter"]		=	"start",
		["lshift"]		=	{"fire","a"},
		["z"]			=	{"fire","b"},
		["x"]			=	{"fire","l1"},
		["lcontrol"]	=	{"fire","x"},
		["lmenu"]		=	{"fire","y"},
		["space"]		=	{"fire","r1"},
		["1"]			=	"l2",
		["esc"]			=	"r2",
	}

	function keys.setup(max_up)
		max_up=max_up or 1
		keys.up={}
		for i=1,max_up do
			keys.up[i]=keys.create(i) -- 1up 2up etc
		end
		
		if max_up==1 then -- single player, grab lots of keys
			if oven.opts.bake.smell=="pimoroni" then
				for n,v in pairs(keys.defaults["pimoroni"]) do
					keys.up[1].set(n,v)
				end
			else
				for n,v in pairs(keys.defaults[0]) do
					keys.up[1].set(n,v)
				end
			end
		else
			for i=1,max_up do -- multiplayer use keyislands so we can all fit on a keyboard
				for n,v in pairs(keys.defaults[i] or {}) do
					keys.up[i].set(n,v)
				end
			end
		end
		
		return keys -- so setup is chainable with a bake
	end


-- convert keys or whatever into recaps changes
	function keys.msg(m)
		if not keys.up then return end -- no key maping

		local used=false
		for i,v in ipairs(keys.up) do
			used=used or v.msg(m)
		end
		return used
	end
	

-- a players key mappings, maybe we need multiple people on the same keyboard or device
	function keys.create(idx)
		local key={}
		key.idx=idx
		key.maps={}
		
		key.joy={}					
		key.joy.class="joystick"
		key.joy.lx=0
		key.joy.rx=0
		key.joy.dx=0
		key.joy.ly=0
		key.joy.ry=0
		key.joy.dy=0

		function key.clear()
			key.maps={}
		end
		function key.set(n,v)
			key.maps[n]=v
		end

		function key.msg(m)
			local used=false
			local recap=key.idx and recaps.up and recaps.up[key.idx]
			if not recap then return end
			
			if m.class=="key" then

				for n,v in pairs(key.maps) do
					if m.keyname==n then
						if m.action==1 then -- key set
							recap.but(v,true)
							used=true
						elseif m.action==-1 then -- key clear
							recap.but(v,false)
							used=true
						end
					end
				end				

			elseif m.class=="posix_joystick" then

				if m.type==1 then -- keys

					if m.value==1 then -- key set
						recap.but("fire",true)
						used=true
					elseif m.value==0 then -- key clear
						recap.but("fire",false)
						used=true
					end

				elseif m.type==3 then -- sticks ( assume ps3 config )
				
					local active=false
					if m.code==0 then
						key.joy.lx=(m.value-128)/128
						active=true
						used=true
					elseif m.code==1 then
						key.joy.ly=(m.value-128)/128
						active=true
						used=true
					elseif m.code==2 then
						key.joy.rx=(m.value-128)/128
						active=true
						used=true
					elseif m.code==5 then
						key.joy.ry=(m.value-128)/128
						active=true
						used=true
					end

					if active then
						local joydir=mkeys.joystick_msg_to_key(key.joy)

						if keys.last_joydir~=joydir then -- only when we change
							if keys.last_joydir then -- first clear any previous key
								recap.but(keys.last_joydir,false)
							end
							keys.last_joydir=joydir
							if joydir then
								recap.but(joydir,true) -- then send any new key
								used=true
							end
						end
					end
				end
			
			elseif m.class=="joystick" then

				local joydir=mkeys.joystick_msg_to_key(m)
				
				-- this does not handle diagonal movement, forces one of 4 directions.

				if keys.last_joydir~=joydir then -- only when we change
--print(wstr.dump(m))
					if keys.last_joydir then -- first clear any previous key
						recap.but(keys.last_joydir,false)
					end
					keys.last_joydir=joydir
					if joydir then
						recap.but(joydir,true) -- then send any new key
						used=true
					end
				end

			elseif m.class=="joykey" then
			
				if m.action==1 then -- key set
					recap.but("fire",true)
					used=true
				elseif m.action==-1 then -- key clear
					recap.but("fire",false)
					used=true
				end

			end
			
			return used -- if we used the msg
		end
		
		return key
	end


	return keys
end

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
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
	keys.defaults[0]={
		["up"]			=	"up",
		["w"]			=	"up",
		["down"]		=	"down",
		["s"]			=	"down",
		["left"]		=	"left",
		["a"]			=	"left",
		["right"]		=	"right",
		["d"]			=	"right",
		["rcontrol"]	=	"fire",
		["rmenu"]		=	"fire",
		["space"]		=	"fire",
		["lcontrol"]	=	"fire",
		["lmenu"]		=	"fire",
	}

	keys.defaults[1]={
		["w"]			=	"up",
		["s"]			=	"down",
		["a"]			=	"left",
		["d"]			=	"right",
		["lcontrol"]	=	"fire",
		["lmenu"]		=	"fire",
	}
	keys.defaults[2]={
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["rcontrol"]	=	"fire",
		["rmenu"]		=	"fire",
	}

	function keys.setup(max_up)
		max_up=max_up or 1
		keys.up={}
		for i=1,max_up do
			keys.up[i]=keys.create(i) -- 1up 2up etc
		end
		
		if max_up==1 then -- single player, grab lots of keys
			for n,v in pairs(keys.defaults[0]) do
				keys.up[1].set(n,v)
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
		if not keys.up then return end
		
		for i,v in ipairs(keys.up) do
			v.msg(m)
		end
		
	end
	

-- a players key mappings, maybe we need multiple people on the same keyboard or device
	function keys.create(idx)
		local key={}
		key.idx=idx
		key.maps={}
		
		function key.clear()
			key.maps={}
		end
		function key.set(n,v)
			key.maps[n]=v
		end

		function key.msg(m)
			local recap=key.idx and recaps.up and recaps.up[key.idx]
			if not recap then return end
			
			if m.class=="key" then

				for n,v in pairs(key.maps) do
					if m.keyname==n then
						if m.action==1 then -- key set
							recap.but(v,true)
						elseif m.action==-1 then -- key clear
							recap.but(v,false)
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
					end
				end

			elseif m.class=="joykey" then
			
				if m.action==1 then -- key set
					recap.but("fire",true)
				elseif m.action==-1 then -- key clear
					recap.but("fire",false)
				end

			end
			
		end
		
		return key
	end


	return keys
end

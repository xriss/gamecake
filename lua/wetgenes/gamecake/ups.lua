--
-- (C) 2024 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")


--[[

Since the very first arcade games the UP in 1UP has always stood for 
*U*ser in*P*ut and this is obvioulsy not a backronym.

This is a replacement for the keys and recaps systems, recaps has never 
really been used as it was intended ( to recored and replay ) and the 
whole thing has gotten rather complicated and entwined with the input 
system. So this is an attemt to simplify all that junk without worrying 
too much about backwards compatibility which I can hack in later as 
necesary.

Ideally we want a way of mapping keys/mouse/gamepads etc into gamepad 
like controls for 1-X players and a player 0 who is all/any gamepad. On 
top of this we need to be able to diff and tween gamepad states for 
network fun.

Finally we need to cache the raw messages for when we are doing 
something even more complicated, eg being a proper gui rather than a 
game and this needs to include a way of moving these msgs onto another 
thread so the data needs to be kept pure.

So yeah, lots of stuff going on here but the basic idea for basic use 
is that we provide.

	ups 0 as the state of all gamepads, eg poke any gamepad or keyboard 
	and all button press will happen here.

	ups X as the state of a single gamepad which requires mapping to 
	hardware gamepads or virtual keyboard gamepads or even a remote 
	gamepad for a networked player.

	A list of all msgs we recieve this frame, everytime we advance a 
	frame this is reset so it *must* be polled every frame update.

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local keymaps={}

-- single player covering entire keyboard controller style
keymaps["full"]={
	["w"]			=	{ "ly0" , "up"    },
	["a"]			=	{ "lx0" , "left"  },
	["s"]			=	{ "ly1" , "down"  },
	["d"]			=	{ "lx1" , "right" },
	["up"]			=	{ "ry0" },
	["left"]		=	{ "rx0" },
	["down"]		=	{ "ry1" },
	["right"]		=	{ "rx1" },
	["keypad 8"]	=	{ "pad_up"    },
	["keypad 4"]	=	{ "pad_left"  },
	["keypad 2"]	=	{ "pad_down"  },
	["keypad 6"]	=	{ "pad_right" },
	["tab"]			=	{ "select" },
	["enter"]		=	{ "start" },
	["z"]			=	{ "l1" },
	["c"]			=	{ "r1" },
	["q"]			=	{ "lz1" , "l2" },
	["e"]			=	{ "rz1" , "r2" },
	["shift_l"]		=	{ "l3" },
	["shift_r"]		=	{ "r3" },
	["enter"]		=	{ "y" , "fire"},
	["ctrl_l"]		=	{ "x" , "fire"},
	["space"]		=	{ "a" , "fire"},
	["ctrl_r"]		=	{ "b" , "fire" },
}


-- 1up shared keyboard
keymaps["island1"]={
	["up"]			=	{ "up" , "pad_up" , "ly0" },
	["down"]		=	{ "down" , "pad_down" , "ly1" },
	["left"]		=	{ "left" , "pad_left" , "lx0" },
	["right"]		=	{ "right" , "pad_right" , "lx1" },
	["."]			=	{ "fire" , "x"},
	["/"]			=	{ "fire" , "x"},
	["shift_r"]		=	{ "fire" , "y"},
	["alt_r"]		=	{ "fire" , "a"},
	["control_r"]	=	{ "fire" , "b"},
	["space"]		=	{ "fire" , "x"},
	["return"]		=	{ "fire" , "a"},
	["enter"]		=	{ "fire" , "b"},
	["["]			=	{ "l1" },
	["]"]			=	{ "r1" },
	["-"]			=	{ "lz1" , "l2"},
	["="]			=	{ "rz2" , "r2"},
	["9"]			=	{ "select"},
	["0"]			=	{ "start"},
}

-- 2up shared keyboard
keymaps["island2"]={
	["w"]			=	{ "up" , "pad_up" , "ly0" },
	["s"]			=	{ "down" , "pad_down" , "ly1" },
	["a"]			=	{ "left" , "pad_left" , "lx0" },
	["d"]			=	{ "right" , "pad_right" , "lx1" },
	["<"]			=	{ "fire" , "y" },
	["z"]			=	{ "fire" , "y" },
	["shift_l"]		=	{ "fire" , "x" },
	["control_l"]	=	{ "fire" , "a" },
	["alt_l"]		=	{ "fire" , "b" },
	["shift"]		=	{ "fire" , "x" }, -- grab all the shift
	["control"]		=	{ "fire" , "a" }, -- control and alt keys
	["alt"]			=	{ "fire" , "b" }, -- if we cant tell left from right
	["z"]			=	{ "l1" },
	["c"]			=	{ "r1" },
	["q"]			=	{ "lz1" , "l2" },
	["e"]			=	{ "rz2" , "r2" },
	["1"]			=	{ "select" },
	["2"]			=	{ "start" },
}

-- merge the island1&2 ups in here
keymaps["islands"]={}
for n,v in pairs(keymaps["island1"]) do keymaps["islands"][n]=v end
for n,v in pairs(keymaps["island2"]) do keymaps["islands"][n]=v end



-- these should be configurable 
local powzone=2			-- walk helper
local minzone=0x1000	-- deadzone
local maxzone=0x7000	-- run helper

local fixaxis=function(n)
	local fix=function(n)
		local n=(n-minzone)/(maxzone-minzone)
		if n<0 then return 0 end
		if n>1 then return 1 end
		return math.pow(n,powzone)
	end
	if type(n)~="number" then n=0 end -- replace nils etc
	if n < 0 then
		return -fix(-n)
	else
		return fix(n)
	end
end

M.up_functions={}
M.up_metatable={__index=M.up_functions}

-- any functions here should not assume that the up we have will have
-- the correct metatable associated with them so we will need to call
-- other up functions explicitly.

M.up_functions.reset=function(up)

	up.all={} -- this frames button state

	up.pulse={} -- "_set" or "_clr" next frame only pulse	
	up.last_pad_values={} -- needed for pad axis buttons

end

-- get stable value ( raw axis will be +-32767 )
M.up_functions.get=function(up,name)
	return up.all[name]
end

-- get stable axis fixed to +-1.0
M.up_functions.axis=function(up,name) -- fix deadzones etc
	local ax=up.all[name]
	if ax then return fixaxis( ax ) end
	return 0
end

M.up_functions.set_button=function(up,n,v)
	local o=up.all[n] -- old value
	if v then -- must be boolean so set
		up.all[nam]=true
		if not o then -- change?
			local nam_set=nam.."_set"
			up.all[nam_set]=true
			up.pulse[nam_set]=true
		end
	else -- or clr
		up.all[nam]=false
		if o then -- change?
			local nam_clr=nam.."_clr"
			up.all[nam_clr]=true
			up.pulse[nam_clr]=true
		end
	end
end

M.up_functions.set_axis=function(up,n,v)
	recap.all[n]=v -- range of -0x7fff to +0x7fff
end

M.up_functions.add_axis=function(up,n,v)
	local o=up.all[n] -- old value
	recap.all[n]=((((o or 0)+v)%0x10000)+0x10000)%0x10000 -- range of 0x0000 to 0xffff
end

-- manage fake axis with decay from keypress states
M.up_functions.fake_axis=function(up)

	for n,a in ipairs{"lx","ly","lz","rx","ry","rz"} do -- check axis buttons and convert to axis movement
		local ak=a.."k"
		local o=(up.all[ak] or 0)
		local v=0
		if     up.all[a.."0"] then v=-32767 if(o>0) then o=0 end
		elseif up.all[a.."1"] then v= 32767 if(o<0) then o=0 end
		end
		if v==0 then -- fast decay to zero
			v=math.floor(o*0.8+v*0.2)
		else -- slow adjust to full
			v=math.floor(o*0.95+v*0.05)
		end
		up:set_axis(ak,v)
	end
	
	-- pick best l/r axis be it fake keys or real pad and copy to "??b" axis
	local lx ,ly ,lz =(up.all["lx"]  or 0),(up.all["ly"]  or 0),(up.all["lz"]  or 0)
	local lxk,lyk,lzk=(up.all["lxk"] or 0),(up.all["lyk"] or 0),(up.all["lzk"] or 0)
	if lxk*lxk+lyk*lyk+lzk*lzk > lx*lx+ly*ly+lz*lz then		up:set_axis("lxb",lxk) up:set_axis("lyb",lyk) up:set_axis("lzb",lzk)
	else													up:set_axis("lxb",lx ) up:set_axis("lyb",ly ) up:set_axis("lzb",lz )
	end
	local rx ,ry ,rz =(up.all["rx"]  or 0),(up.all["ry"]  or 0),(up.all["rz"]  or 0)
	local rxk,ryk,rzk=(up.all["rxk"] or 0),(up.all["ryk"] or 0),(up.all["rzk"] or 0)
	if rxk*rxk+ryk*ryk+rzk*rzk > rx*rx+ry*ry+rz*rz then		up:set_axis("rxb",rxk) up:set_axis("ryb",ryk) up:set_axis("rzb",rzk)
	else													up:set_axis("rxb",rx ) up:set_axis("ryb",ry ) up:set_axis("rzb",rz )
	end
end

-- make pulses only last one update
M.up_functions.update=function(up)

	for n,v in pairs(up.pulse) do
		up.all[n]=v -- set or clear this frame
		if v then	up.pulse[n]=false	-- clear next frame
		else		up.pulse[n]=nil		-- then forget
		end
	end

end
		
M.bake=function(oven,ups)

	ups=ups or {}
	
	ups.reset=function()
		ups.last_pad_values={}
		ups.keymaps={}
		ups.states={}
		ups.all_msgs={} -- all the msgs for this tick
		ups.now_msgs={} -- volatile building list of msgs
	end

	ups.keymap=function(idx,map)
		if map then
			ups.keymaps[idx]={} -- reset
			if keymaps[map] then map=keymaps[map] end -- named map
			for n,v in pairs(map) do -- copy
				ups.keymap[n]={unpack(v)}
			end
		end
		return ups.keymaps[idx]
	end

	-- get or create a state and remember its idx in states
	ups.manifest=function(idx)
		if ups.states[idx] then return ups.states[idx] end -- already created
		ups.states[idx]=ups.create() -- create and remember
		return ups.states[idx]
	end
	-- create a state
	ups.create=function()
		local up={}
		setmetatable(up,M.up_metatable) -- shared functions
		up:reset() -- initalize
		return up
	end

-- this is called as each msg is recieved and should be as fast as possible
-- so just copy the msg into our volatile state for processing on next update
	ups.msg=function(mm)
		local m={} -- we will copy and cache
		for n,v in pairs(mm) do m[n]=v end -- copy top level only
		ups.now_msgs[#ups.now_msgs+1]=m -- remember
	end
	
	ups.msg_apply=function(m)
		if m.class=="key" then
		
			for idx,maps in ipairs(ups.keymaps) do -- check all keymaps
				local buttons=maps[m.keyname] or maps[m.ascii]
				if buttons then
					local up=ups.mainfest(idx)
					if m.action==1 then -- key set
						for _,button in ipairs(buttons) do
							up:set_button(button,true)
						end
					elseif m.action==-1 then -- key clear
						for _,button in ipairs(buttons) do
							up:set_button(button,false)
						end
					end
				end
			end
			
		elseif m.class=="mouse" then -- swipe to move
		
			local up=ups.mainfest(1) -- mouse is 1up only

			if m.action==-1 then -- we only get button ups
				if m.keyname=="wheel_add" then
					up:add_axis( "mz" , 1 )
				elseif m.keyname=="wheel_sub" then
					up:add_axis( "mz" , -1 )
				end
			end

			up:add_axis( "mx" , m.dx )
			up:add_axis( "my" , m.dy )
			up:set_axis( "vx" , m.x )
			up:set_axis( "vy" , m.y )

			if m.action==1 then -- key set
				if m.keyname then
					up:set_button("mouse_"..m.keyname,true)
				end
				if m.keyname=="left" then
					up:set_button("fire",true)
				end
			elseif m.action==-1 then -- key clear
				if m.keyname=="wheel_add" or m.keyname=="wheel_sub" then -- we do not get key downs just ups
					up:set_button("mouse_"..m.keyname,true)
				end
				if m.keyname then
					up:set_button("mouse_"..m.keyname,false)
				end
				if m.keyname=="left" then
					up:set_button("fire",false)
				end
			end

			elseif m.class=="padaxis" then -- SDL axis values

				local up=ups.mainfest(m.id) -- this pad belongs to

				local zone=0x2000

				local doaxis=function(name1,name2)

					local v=0
					if     m.value <= -zone then v=-1
					elseif m.value >=  zone then v= 1
					end

					if up.last_pad_value[name1]~=v then -- only on change
						if     v<0 then	up:set_button(name1,true)	up:set_button(name2,false)
						elseif v>0 then	up:set_button(name1,false)	up:set_button(name2,true)
						else			up:set_button(name1,false)	up:set_button(name2,false)
						end
					end

					up.last_pad_value[name1]=v
				end

				local dotrig=function(name)

					local v=0
					if m.value >=  zone then v= 1
					end

					if up.last_pad_value[name]~=v then -- only on change

						if     v>0 then		up:set_button(name,true)
						else				up:set_button(name,false)
						end
					end

					up.last_pad_value[name]=v
				end
				
				if     m.name=="LeftX"			then		doaxis("left","right")	up:set_axis("lx",m.value)
				elseif m.name=="LeftY"			then		doaxis("up","down")		up:set_axis("ly",m.value)
				elseif m.name=="RightX"			then		doaxis("left","right")	up:set_axis("rx",m.value)
				elseif m.name=="RightY"			then		doaxis("up","down")		up:set_axis("ry",m.value)
				elseif m.name=="TriggerLeft"	then		dotrig("l2")			up:set_axis("lz",m.value)
				elseif m.name=="TriggerRight"	then		dotrig("r2")			up:set_axis("rz",m.value)
				end

			elseif m.class=="padkey" then -- SDL button values

				local up=ups.mainfest(m.id) -- this pad belongs to

				local docode=function(name)
					if		m.value==1  then	up:set_button(name,true)	-- key set
					elseif	m.value==-1 then	up:set_button(name,false)	-- key clear
					end
				end

				if     m.name=="Left"			then docode("left")   docode("pad_left")    up:set_axis("dx",(m.value==1) and -32767 or 0)
				elseif m.name=="Right"			then docode("right")  docode("pad_right")   up:set_axis("dx",(m.value==1) and  32767 or 0)
				elseif m.name=="Up"				then docode("up")     docode("pad_up")      up:set_axis("dy",(m.value==1) and -32767 or 0)
				elseif m.name=="Down"			then docode("down")   docode("pad_down")    up:set_axis("dy",(m.value==1) and  32767 or 0)
				elseif m.name=="A"				then docode("a")      docode("fire")
				elseif m.name=="B"				then docode("b")      docode("fire")
				elseif m.name=="X"				then docode("x")      docode("fire")
				elseif m.name=="Y"				then docode("y")      docode("fire")
				elseif m.name=="LeftShoulder"	then docode("l1")
				elseif m.name=="RightShoulder"	then docode("r1")
				elseif m.name=="LeftStick"		then docode("l3")
				elseif m.name=="RightStick"		then docode("r3")
				elseif m.name=="Back"			then docode("select")
				elseif m.name=="Start"			then docode("start")
				elseif m.name=="Guide"			then docode("guide")
				else docode("fire") -- all other buttons are fire
				end
				
			end
		end
	end

	ups.update=function()
	
		if oven.is.update then -- pull all msgs from other thread

			m.all_msgs={} -- about to fill this
			m.now_msgs={} -- this is not used
			repeat
				local ok,m=oven.tasks.linda:receive(0,"ups") -- grab all available memos
				if ok and m then
					for _,v in ipairs(m.all_msgs) do
						ups.all_msgs[#ups.all_msgs+1]=v
					end
				end
			until not m
			
		else

			-- swap now to all
			ups.all_msgs=ups.now_msgs
			ups.now_msgs={}

			if oven.is.main then -- push all msgs to other thread

				local m={}
				m.all_msgs=ups.all_msgs
				oven.tasks.linda:send(nil,"ups",m)
			
			end

		end

		-- change current state using all msgs
		for _,m in ipairs( ups.all_msgs ) do
			ups.msg_apply(m)
		end

		-- advance each state one frame
		for idx,up in pairs(ups.states) do
			up:fake_axis()
			up:update()
		end

	end

	-- create 1up only by default
	ups.reset(1)
	ups.manifest(1)
	ups.keymap(1,"full")

	return ups
end

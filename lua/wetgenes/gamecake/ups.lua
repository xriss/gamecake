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

	ups X as the state of a single gamepad which requires mapping to
	hardware gamepads or virtual keyboard gamepads or even a remote
	gamepad for a networked player.

	by default we will map mouse and keys to up1 so that can be used
	where keyboard and mouse values are wanted.

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
	["control_l"]	=	{ "x" , "fire"},
	["space"]		=	{ "a" , "fire"},
	["control_r"]	=	{ "b" , "fire" },
}

-- basic 1up with only a single fire button
keymaps["basic"]={
	["up"]			=	{ "up" , "pad_up" , "ly0" },
	["down"]		=	{ "down" , "pad_down" , "ly1" },
	["left"]		=	{ "left" , "pad_left" , "lx0" },
	["right"]		=	{ "right" , "pad_right" , "lx1" },
	["shift_r"]		=	{ "fire" },
	["alt_r"]		=	{ "fire" },
	["control_r"]	=	{ "fire" },
	["shift_l"]		=	{ "fire" },
	["alt_l"]		=	{ "fire" },
	["control_l"]	=	{ "fire" },
	["space"]		=	{ "fire" },
	["return"]		=	{ "fire" },
	["enter"]		=	{ "fire" },
	["shift"]		=	{ "fire" }, -- also grab all the shift
	["control"]		=	{ "fire" }, -- control and alt keys
	["alt"]			=	{ "fire" }, -- if we cant tell left from right
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
	["shift"]		=	{ "fire" }, -- also grab all the shift
	["control"]		=	{ "fire" }, -- control and alt keys
	["alt"]			=	{ "fire" }, -- if we cant tell left from right
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
	["z"]			=	{ "l1" },
	["c"]			=	{ "r1" },
	["q"]			=	{ "lz1" , "l2" },
	["e"]			=	{ "rz2" , "r2" },
	["1"]			=	{ "select" },
	["2"]			=	{ "start" },
	["shift"]		=	{ "fire" }, -- also grab all the shift
	["control"]		=	{ "fire" }, -- control and alt keys
	["alt"]			=	{ "fire" }, -- if we cant tell left from right
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

-- names of relative mouse axis
M.is_relative={mx=true,my=true,mz=true}
M.is_pulse={}

for i,n in ipairs{
		"left","pad_left","right","pad_right","up","pad_up","down","pad_down",
		"a","b","x","y",	"l1","r1","l2","r2","l3","r3",	"select","start","guide","fire",
		} do
	M.is_pulse[ n.."_set" ] = true
	M.is_pulse[ n.."_clr" ] = true
end

M.up_functions={}
M.up_metatable={__index=M.up_functions}

-- any functions here should not assume that the up we have will have
-- the correct metatable associated with them so we will need to call
-- other up functions explicitly.

M.up_functions.reset=function(up)

	up.all={} -- this frames button state

end

-- get stable value ( raw axis will be +-32767 )
M.up_functions.get=function(up,name)
	return up.all[name]
end

-- get stable axis fixed to +-1.0
M.up_functions.axis=function(up,name) -- fix deadzones etc
	return fixaxis( up.all[name] )
end

M.up_functions.set_button=function(up,n,v)
	local o=up.all[n] -- old value
	if v then -- must be boolean so set
		up.all[n]=true
		if not o then -- change?
			local n_set=n.."_set"
			up.all[n_set]=true
		end
	else -- or clr
		up.all[n]=false
		if o then -- change?
			local n_clr=n.."_clr"
			up.all[n_clr]=true
		end
	end
end

M.up_functions.set_axis=function(up,n,v)
	up.all[n]=v -- raw range of -0x7fff to +0x7fff probably
end

M.up_functions.add_axis=function(up,n,v)
	local o=up.all[n] or 0-- old value
	up.all[n]=o+v
end


-- manage fake axis with decay from keypress states
-- and handle deletion of pulse msgs
M.up_functions.update=function(up,pow)
	pow=64*(pow or 1/60)

	for n,a in ipairs{"lx","ly","lz","rx","ry","rz"} do -- check axis buttons and convert to axis movement
		local ak=a.."k"
		local o=(up.all[ak] or 0)
		local v=0
		if     up.all[a.."0"] then v=-32767 if(o>0) then o=0 end
		elseif up.all[a.."1"] then v= 32767 if(o<0) then o=0 end
		end
		local s=o<0 and -1 or 1 -- remove sign
		if v==0 then -- fast decay to zero
			local p=0.80^pow
			v=math.floor((o*p        )*s)*s
		else -- slower ramp to full
			local p=0.95^pow
			v=math.floor((o*p+v*(1-p))*s)*s
		end
		up.all[ak]=v
	end

	-- pick best l/r axis be it "k"eys or "p"ad values.
	local lxp,lyp,lzp=(up.all["lxp"] or 0),(up.all["lyp"] or 0),(up.all["lzp"] or 0)
	local lxk,lyk,lzk=(up.all["lxk"] or 0),(up.all["lyk"] or 0),(up.all["lzk"] or 0)
	if lxk*lxk+lyk*lyk > lxp*lxp+lyp*lyp then -- stick
		up.all["lx"]=lxk ; up.all["ly"]=lyk
	else
		up.all["lx"]=lxp ; up.all["ly"]=lyp
	end
	if lzk*lzk > lzp*lzp then -- trigger
		up.all["lz"]=lzk
	else
		up.all["lz"]=lzp
	end
	local rxp,ryp,rzp=(up.all["rxp"] or 0),(up.all["ryp"] or 0),(up.all["rzp"] or 0)
	local rxk,ryk,rzk=(up.all["rxk"] or 0),(up.all["ryk"] or 0),(up.all["rzk"] or 0)
	if rxk*rxk+ryk*ryk > rxp*rxp+ryp*ryp then -- stick
		up.all["rx"]=rxk ; up.all["ry"]=ryk
	else
		up.all["rx"]=rxp ; up.all["ry"]=ryp
	end
	if rzk*rzk > rzp*rzp then -- trigger
		up.all["rz"]=rzk
	else
		up.all["rz"]=rzp
	end

	-- auto delete pulses
	for n,v in pairs(up.all) do
		if M.is_pulse[n] then
			if up.all[n]==1 then
				up.all[n]=nil
			else
				up.all[n]=1	-- use one as a pulse flag that is also true
			end
		end
	end

end

-- save to json ( sameish data as an up but no metatable )
M.up_functions.save=function(up,r)
	local r=r or {}

	if next(up.all) then -- something
		r.all={}
		for n,v in pairs(up.all) do
			r.all[n]=v
		end
	end

	return r
end

-- load from saved json or another up as they are similar
M.up_functions.load=function(up,r)

	for n,v in pairs(up.all) do -- empty
		up.all[n]=nil
	end
	if r and r.all then -- something
		for n,v in pairs(r.all) do
			up.all[n]=v
		end
	end

end

-- remove pulse flags and relative movement
M.up_functions.unpulse=function(up)
	-- mouse
	up.all.mx=nil
	up.all.my=nil
	up.all.mz=nil
	-- auto delete pulses
	for n,v in pairs(up.all) do
		if M.is_pulse[n] then -- remove all pulses
			up.all[n]=nil
		end
	end
end

-- merge from saved json or another up as they are similar
M.up_functions.merge=function(up,r)

	if r and r.all then -- something
		for n,v in pairs(r.all) do
			if M.is_pulse[n] then
				up.all[n]=up.all[n] or v
			elseif M.is_relative[n] then -- special merge add for mouse axis
				up.all[n] = ( up.all[n] or 0 ) + v
			else
				up.all[n]=v
			end
		end
	end

end

-- create new up when we have another up,
M.up_functions.create=function()
	local up={}
	setmetatable(up,M.up_metatable) -- shared functions
	up:reset() -- initalize
	return up
end

-- duplicate this up
M.up_functions.duplicate=function(up)
	local n=up.create()
	n:load(up)
	return n
end

-- global empty ups
-- please do not write into it
M.empty=M.up_functions.create()

M.bake=function(oven,ups)

	ups=ups or {}

	ups.empty=M.empty

	ups.auto_advance=true -- automatically advance each update
	-- upnet will turn this off so it can network sync advances

	-- reset everything
	ups.reset=function()
		ups.last_pad_values={}
		ups.keymaps={}
		ups.mousemaps={1}
		ups.padmaps={1}
		ups.states={}
		ups.msgs={} -- all the msgs for this tick
		ups.new_msgs={} -- volatile building list of msgs

		ups.enable_pad=true		-- deal with pad msgs or ignore them (grabbed by somone else)
		ups.enable_key=true		-- deal with keyboard msgs or ignore them (grabbed by somone else)
		ups.enable_mouse=true	-- deal with mouse msgs or ignore them (grabbed by somone else)
	end

	-- set keymap for this idx
	ups.keymap=function(idx,map)
		if map then
			ups.keymaps[idx]={} -- reset
			local m=ups.keymaps[idx]
			if keymaps[map] then map=keymaps[map] end -- named map
			for n,v in pairs(map) do -- copy
				m[n]={unpack(v)}
			end
		end
		return ups.keymaps[idx]
	end

	-- set mousemap to this idx
	ups.mousemap=function(...)
		ups.mousemaps={...}
	end

	-- set padmap to these idxs
	ups.padmap=function(...)
		ups.padmaps={...}
	end


	-- get or create a state and remember its idx in states
	ups.manifest=function(idx)
		if idx<1 then return nil end -- asking for 0 gets you a nil
		if ups.states[idx] then return ups.states[idx] end -- already created
		ups.states[idx]=ups.create() -- create and remember
		return ups.states[idx]
	end

	-- create a state
	ups.create=M.up_functions.create

-- this is called as each msg is recieved and should be as fast as possible
-- so just copy the msg into our volatile state for processing on next update
	ups.msg=function(mm)
		local m={} -- we will copy and cache
		for n,v in pairs(mm) do m[n]=v end -- copy top level only
		ups.new_msgs[#ups.new_msgs+1]=m -- remember
	end

	-- apply msgs to button states
	ups.msg_apply=function(m)
		if m.class=="key" and ups.enable_key then
			for idx,maps in ipairs(ups.keymaps) do -- check all keymaps
				local buttons=maps[m.keyname] or maps[m.ascii]
				if buttons then
					local up=ups.manifest(idx)
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

		elseif m.class=="mouse" and ups.enable_mouse then -- swipe to move

			local up=ups.manifest( ups.mousemaps[1] ) -- probably 1up
			if up then
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
						up:set_button("mouse_"..m.keyname,true) -- fake a down to force a pulse
					end
					if m.keyname then
						up:set_button("mouse_"..m.keyname,false)
					end
					if m.keyname=="left" then
						up:set_button("fire",false)
					end
				end
			end
		elseif m.class=="padaxis" and ups.enable_pad then -- SDL axis values

			local upi=ups.padmaps[((m.id-1)%#ups.padmaps)+1]
			if not ups.last_pad_values[upi] then ups.last_pad_values[upi]={} end -- manifest
			local last_pad_values=ups.last_pad_values[upi]
			local up=ups.manifest( upi ) -- this pad belongs to
			if up then
				local zone=0x2000

				local doaxis=function(name1,name2)

					local v=0
					if     m.value <= -zone then v=-1
					elseif m.value >=  zone then v= 1
					end

					if last_pad_values[name1]~=v then -- only on change
						if     v<0 then	up:set_button(name1,true)	up:set_button(name2,false)
						elseif v>0 then	up:set_button(name1,false)	up:set_button(name2,true)
						else			up:set_button(name1,false)	up:set_button(name2,false)
						end
					end

					last_pad_values[name1]=v
				end

				local dotrig=function(name)

					local v=0
					if m.value >=  zone then v= 1
					end

					if last_pad_values[name]~=v then -- only on change

						if     v>0 then		up:set_button(name,true)
						else				up:set_button(name,false)
						end
					end

					last_pad_values[name]=v
				end

				if     m.name=="LeftX"			then		doaxis("left","right")	up:set_axis("lxp",m.value)
				elseif m.name=="LeftY"			then		doaxis("up","down")		up:set_axis("lyp",m.value)
				elseif m.name=="RightX"			then		doaxis("left","right")	up:set_axis("rxp",m.value)
				elseif m.name=="RightY"			then		doaxis("up","down")		up:set_axis("ryp",m.value)
				elseif m.name=="TriggerLeft"	then		dotrig("l2")			up:set_axis("lzp",m.value)
				elseif m.name=="TriggerRight"	then		dotrig("r2")			up:set_axis("rzp",m.value)
				end
			end

		elseif m.class=="padkey" and ups.enable_pad then -- SDL button values

			local upi=ups.padmaps[((m.id-1)%#ups.padmaps)+1]
			local up=ups.manifest( upi ) -- this pad belongs to
			if up then

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

			ups.msgs={} -- about to fill this
			ups.new_msgs={} -- this should never contain anything
			repeat
				local ok,m=oven.tasks.linda:receive(0,"ups") -- grab all available memos
				if ok and m then
					for _,v in ipairs(m.msgs) do
						ups.msgs[#ups.msgs+1]=v
					end
				end
			until not m

		else

			-- swap now to all
			ups.msgs=ups.new_msgs
			ups.new_msgs={} -- start again

			if oven.is.main then -- push all msgs to other thread

				local m={}
				m.msgs=ups.msgs
				oven.tasks.linda:send(nil,"ups",m)

			end

		end

		-- reset relative mouse
		for idx,up in pairs(ups.states) do
			up.all.mx=nil
			up.all.my=nil
			up.all.mz=nil
		end

		-- change current state using all msgs
		for _,m in ipairs( ups.msgs ) do
			ups.msg_apply(m)
		end

		if ups.auto_advance then
			ups.advance()
		end

	end

	-- needed for manual advance
	ups.advance=function(idx)
		-- advance each up state one frame deals with key acc and decay code
		for idx,up in pairs(ups.states) do
			up:update()
		end
	end


	-- old style recaps hacks
	ups.up=function(idx)
		if type(idx)~="number" then idx=1 end
		if idx<1 then idx=1 end -- simpler than wasting time merging every state

		local up={}
		up.core=ups.manifest(idx)
		setmetatable(up,{__index=up.core})

		up.button=function(name)
			return up:get(name)
		end

		up.axis=function(name)
			return up:get(name)
		end

		up.axisfixed=function(name)
			return up:axis(name)
		end

		up.msgs=function(name)
			return ups.msgs
		end

		return up
	end

	-- create 1up only by default
	ups.reset()
	ups.keymap(1,"full") -- 1up has full keyboard mappings
	ups.mousemap(1) -- 1up has the mouse buttons
	ups.padmap(1) -- 1up has all the pads
	ups.manifest(1) -- force 1up to exist

	return ups
end

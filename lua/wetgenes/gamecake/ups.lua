--
-- (C) 2024 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")


--[[

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
	["tab"]			=	"select",
	["enter"]		=	"start",
	["z"]			=	"l1",
	["c"]			=	"r1",
	["q"]			=	{ "lz1" , "l2" },
	["e"]			=	{ "rz1" , "r2" },
	["shift_l"]		=	"l3",
	["shift_r"]		=	"r3",
	["enter"]		=	{ "y" , "fire"},
	["ctrl_l"]		=	{ "x" , "fire"},
	["space"]		=	{ "a" , "fire"},
	["ctrl_r"]		=	{ "b" , "fire" },
}


-- 1up shared keyboard
keymaps["island1"]={
	["up"]			=	{"up","pad_up","ly0"},
	["down"]		=	{"down","pad_down","ly1"},
	["left"]		=	{"left","pad_left","lx0"},
	["right"]		=	{"right","pad_right","lx1"},
	["."]			=	{"fire","x"},
	["/"]			=	{"fire","x"},
	["shift_r"]		=	{"fire","y"},
	["alt_r"]		=	{"fire","a"},
	["control_r"]	=	{"fire","b"},
	["space"]		=	{"fire","x"},
	["return"]		=	{"fire","a"},
	["enter"]		=	{"fire","b"},
	["["]			=	"l1",
	["]"]			=	"r1",
	["-"]			=	{"lz1","l2"},
	["="]			=	{"rz2","r2"},
	["9"]			=	"select",
	["0"]			=	"start",
}

-- 2up shared keyboard
keymaps["island2"]={
	["w"]			=	{"up","pad_up","ly0"},
	["s"]			=	{"down","pad_down","ly1"},
	["a"]			=	{"left","pad_left","lx0"},
	["d"]			=	{"right","pad_right","lx1"},
	["<"]			=	{"fire","y"},
	["z"]			=	{"fire","y"},
	["shift_l"]		=	{"fire","x"},
	["control_l"]	=	{"fire","a"},
	["alt_l"]		=	{"fire","b"},
	["shift"]		=	{"fire","x"}, -- grab all the shift
	["control"]		=	{"fire","a"}, -- control and alt keys
	["alt"]			=	{"fire","b"}, -- if we cant tell left from right
	["z"]			=	"l1",
	["c"]			=	"r1",
	["q"]			=	{"lz1","l2"},
	["e"]			=	{"rz2","r2"},
	["1"]			=	"select",
	["2"]			=	"start",
}

-- merge the island1&2 ups in here
keymaps["islands"]={}
for n,v in pairs(keymaps["island1"]) do keymaps["islands"][n]=v end
for n,v in pairs(keymaps["island2"]) do keymaps["islands"][n]=v end



-- these should be configurable 
local powzone=2				-- walk helper
local minzone=4095			-- deadzone
local maxzone=32767-4096	-- run helper

local fixaxis=function(n)
	local fix=function(n)
		local n=(n-minzone)/(maxzone-minzone)
		if n<0 then return 0 end
		if n>1 then return 1 end
		return math.pow(n,powzone)
	end
	n=n or 0
	if n < 0 then
		return -fix(-n)
	else
		return fix(n)
	end
end

			
			
M.bake=function(oven,ups)

	ups=ups or {}
	
	local cake=oven.cake
	local canvas=cake.canvas
	
	ups.keymaps={}
	ups.keymap=function(idx,map)
		if map then
			ups.keymaps[idx]={} -- reset
			if keymaps[map] then map=keymaps[map] end -- named map
			for n,v in pairs(map) do -- copy
				if type(v)=="table" then v={unpack(v)} end -- dupe
				ups.keymap[n]=v
			end
		end
		return ups.keymaps[idx]
	end

	-- keep track of keyboard qualifiers
	ups.now_qualifiers={false,false,false,false} -- flags of "alt" , "ctrl" , "shift" , "win"  in 1,2,3,4 so we know which keys are held down
	ups.now_qualifiers_text=nil -- string of the above flags joined by + or nil if no curretn qualifiers
	
	local build_qualifiers_text=function()
		local s
		local add=function(key) if key then if s then s=s.."+"..key else s=key end end end
		add(ups.now_qualifiers[1])
		add(ups.now_qualifiers[2])
		add(ups.now_qualifiers[3])
		add(ups.now_qualifiers[4])
		ups.now_qualifiers_text=s
	end
	function ups.msg_set_qualifiers(m)
		if not m then return end
		if m.class=="key" then
			if     ( m.keyname=="alt" or m.keyname=="alt_l" or m.keyname=="alt_r" ) then
				if     m.action== 1 then ups.now_qualifiers[1]="alt"
				elseif m.action==-1 then ups.now_qualifiers[1]=false end
				build_qualifiers_text()
			elseif ( m.keyname=="control" or m.keyname=="control_l" or m.keyname=="control_r" ) then
				if     m.action== 1 then ups.now_qualifiers[2]="ctrl"
				elseif m.action==-1 then ups.now_qualifiers[2]=false end
				build_qualifiers_text()
			elseif ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) then
				if     m.action== 1 then ups.now_qualifiers[3]="shift"
				elseif m.action==-1 then ups.now_qualifiers[3]=false end
				build_qualifiers_text()
			elseif ( m.keyname=="gui" or m.keyname=="left gui" or m.keyname=="right gui" ) then -- gui/command/windows key?
				if     m.action== 1 then ups.now_qualifiers[4]="win"
				elseif m.action==-1 then ups.now_qualifiers[4]=false end
				build_qualifiers_text()
			end
			m.qualifiers=ups.now_qualifiers_text -- add our cached qualifiers to key messages
		elseif m.class=="mouse" then
			m.qualifiers=ups.now_qualifiers_text -- add our cached qualifiers to mouse messages ( so we can easily catch ctrl click etc )
		end
		return m
	end


	
--[[
	function ups.update()
	end


	function ups.set_opts(n,v)
		if n=="typing" and ups.opts.notyping then v=false end -- disable typing../
		if n=="typing" and ups.opts.yestyping then v=true end -- disable non typing../
		ups.opts[n]=v
	end

-- convert ups or whatever into recaps changes
	function ups.msg(m)

		recaps.ups(1).set_msg(m) -- copy msg

		if not ups.up then return end -- no key maping
		if m.sups then return end -- already processed

		local used=false
		for i,v in ipairs(ups.up) do
			local t=v.msg(m)
			used=used or t
		end
		if used then m.sups=true end -- flag as used by sups
		return used
	end
	

-- a players key mappings, maybe we need multiple people on the same keyboard or device
	function ups.create(idx,opts)
		local key={}
		key.opts=opts or {}
		key.idx=idx
		key.maps={}
		
		key.last_pad_value={} -- cached data to help ignore wobbling inputs
		
		function key.clear()
			key.maps={}
		end
		function key.set(n,v)
			key.maps[n]=v
		end

		function key.msg(m)
		
			local used=false
			local ups=recaps.ups(key.idx)
--			if not ups then return end -- nowhere to send the data
			
			local new_joydir=function(joydir)
					-- handle diagonal movement
					local map={
						["left"]={"left"},
						["right"]={"right"},
						["up"]={"up"},
						["down"]={"down"},
						["up_left"]={"up","left"},
						["up_right"]={"up","right"},
						["down_left"]={"down","left"},
						["down_right"]={"down","right"},
					}
					
					if key.last_joydir~=joydir then -- only when we change direction
						local setclr={}
						if key.last_joydir then -- clear all old ups
							for _,n in ipairs(map[key.last_joydir]  ) do setclr[n]="clr" end
							used=true
						end
						key.last_joydir=joydir
						if joydir then -- set all new ups
							for _,n in ipairs(map[joydir]  ) do setclr[n]="set" end
							used=true
						end
						for n,v in pairs(setclr) do -- send new state (sets may have replaced clrs)
							if v=="clr" then ups.set_button(n,false) end
							if v=="set" then ups.set_button(n,true) end
						end
					end
			end

			if m.class=="key" then

				if not key.opts.typing then -- sometimes we need the keyboard for typing
					for n,v in pairs(key.maps) do
						if m.keyname==n or m.ascii==n then
							if m.action==1 then -- key set
								ups.set_button(v,true)
								used=true
							elseif m.action==-1 then -- key clear
								ups.set_button(v,false)
								used=true
							end
						end
					end
				end

			elseif m.class=="touch" then -- touch areas

				if key.idx==1 then -- only for player 1

					ups.set_axis({tx=m.x,ty=m.y}) -- tell recap about the touch positions, tx,ty
					if m.action==1 then -- key set
						ups.set_button("touch",true)
					elseif m.action==-1 then -- key clear
						ups.set_button("touch",false)
					end

					if ups.touch == "left_right" then -- use a left/right + fire button control system
					
						local x=m.xraw/oven.win.width
						if m.action>0 then
							if x<=1/2 then				ups.set_button("left",true)
							else						ups.set_button("right",true)
							end
						elseif m.action<0 then
							if x<=1/2 then				ups.set_button("left",false)
							else						ups.set_button("right",false)
							end
						end
					
					elseif ups.touch == "left_fire_right" then -- use a left/right + fire button control system
					
						local x=m.xraw/oven.win.width
						if m.action>0 then
							if x<=1/3 then				ups.set_button("left",true)
							elseif x<=2/3 then			ups.set_button("fire",true)
							else						ups.set_button("right",true)
							end
						elseif m.action<0 then
							if x<=1/3 then				ups.set_button("left",false)
							elseif x<=2/3 then			ups.set_button("fire",false)
							else						ups.set_button("right",false)
							end
						end
					
					end
				end
				
			elseif m.class=="mouse" then -- swipe to move
			
				if key.idx==1 then -- only for player 1


					local mz
					if m.action==-1 then -- we only get button ups
						if m.keyname=="wheel_add" then
							mz=1
						elseif m.keyname=="wheel_sub" then
							mz=-1
						end
					end


					ups.set_axis_relative( {mx=m.dx,my=m.dy,mz=mz} ) -- tell recap about the mouse positions, mx,my
					ups.set_axis( {vx=m.x,vy=m.y} ) -- tell recap about the absolute mouse positions in default view

					if m.action==1 then -- key set
						if m.keyname then ups.set_button("mouse_"..m.keyname,true) end
						if m.keyname=="left" or m.keyname=="right" --[=[or m.keyname=="middle"]=] then
							ups.set_button("fire",true)
						end
						used=true
					elseif m.action==-1 then -- key clear
						if m.keyname=="wheel_add" or m.keyname=="wheel_sub" then -- we do not get key downs just ups
							ups.set_button("mouse_"..m.keyname,true)
						end
						if m.keyname then ups.set_button("mouse_"..m.keyname,false) end
						if m.keyname=="left" or m.keyname=="right" --[=[or m.keyname=="middle"]=] then
							ups.set_button("fire",false)
						end
						used=true
					end

-- swipe to keypress code
-- this can be a problem in menus, so is best to only turn it on during gameplay

					if key.opts.swipe then -- use mouse to swipe
					
						if m.action==1 then -- click
							key.swipe={m.x,m.y,m.x,m.y}
						elseif m.action==-1 then -- release
							key.swipe=nil
						elseif m.action==0 then --move
							if key.swipe then
								key.swipe[3]=m.x
								key.swipe[4]=m.y
							end
						end

						if key.swipe then
							local function acc() key.swipe[1]=key.swipe[3]  key.swipe[2]=key.swipe[4] end
							local x=key.swipe[3]-key.swipe[1]
							local y=key.swipe[4]-key.swipe[2]
							local xx=x*x
							local yy=y*y
							local joydir=nil
							if xx+yy > 8*8 then
								if xx > yy then
									if x>=0 then
										joydir="right"
										acc()
									else
										joydir="left"
										acc()
									end
								else
									if y>=0 then
										joydir="down"
										acc()
									else
										joydir="up"
										acc()
									end
								end
							end	
							new_joydir(joydir) -- swipes are single taps
							new_joydir()
						end
					else
						key.swipe=nil
					end
				end
				
			elseif m.class=="padaxis" then -- SDL axis values

				if m.id and key.idx-1 == (m.id+key.opts.pad_map-1)%key.opts.max_up then
					used=true

					local zone=32768/4

					local doaxis=function(name1,name2)

						local v=0
						if     m.value <= -zone then v=-1
						elseif m.value >=  zone then v= 1
						end

						if key.last_pad_value[name1]~=v then -- only on change
							if     v<0 then	ups.set_button(name1,true)	ups.set_button(name2,false)
							elseif v>0 then	ups.set_button(name1,false)	ups.set_button(name2,true)
							else			ups.set_button(name1,false)	ups.set_button(name2,false)
							end
						end

						key.last_pad_value[name1]=v
					end

					local dotrig=function(name)

						local v=0
						if m.value >=  zone then v= 1
						end

						if key.last_pad_value[name]~=v then -- only on change

							if     v>0 then		ups.set_button(name,true)
							else				ups.set_button(name,false)
							end
						end

						key.last_pad_value[name]=v
					end
					if     m.name=="LeftX"			then		doaxis("left","right")	ups.set_axis{lx=m.value}
					elseif m.name=="LeftY"			then		doaxis("up","down")		ups.set_axis{ly=m.value}
					elseif m.name=="RightX"			then		doaxis("left","right")	ups.set_axis{rx=m.value}
					elseif m.name=="RightY"			then		doaxis("up","down")		ups.set_axis{ry=m.value}
					elseif m.name=="TriggerLeft"	then		dotrig("l2")			ups.set_axis{lz=m.value}
					elseif m.name=="TriggerRight"	then		dotrig("r2")			ups.set_axis{rz=m.value}
					end

				end

			elseif m.class=="padkey" then -- SDL button values

				if m.id and key.idx-1 == (m.id+key.opts.pad_map-1)%key.opts.max_up then
					used=true

					local docode=function(name)
						if		m.value==1  then	ups.set_button(name,true)	-- key set
						elseif	m.value==-1 then	ups.set_button(name,false)	-- key clear
						end
					end

					if     m.name=="Left"			then docode("left")   docode("pad_left")    ups.set_axis{dx=(m.value==1) and -32768 or 0}
					elseif m.name=="Right"			then docode("right")  docode("pad_right")   ups.set_axis{dx=(m.value==1) and  32767 or 0}
					elseif m.name=="Up"				then docode("up")     docode("pad_up")      ups.set_axis{dy=(m.value==1) and -32768 or 0}
					elseif m.name=="Down"			then docode("down")   docode("pad_down")    ups.set_axis{dy=(m.value==1) and  32767 or 0}
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
					else docode("fire") -- other buttons are fire
					end
					
				end

			end
			
			return used -- if we used the msg
		end
		
-- manage fake axis and decay from keypress states
		key.update=function()
			local ups=recaps.ups(key.idx)
			for n,a in ipairs{"lx","ly","lz","rx","ry","rz"} do -- check axis buttons and convert to axis movement
				local ak=a.."k"
				local o=(ups.get(ak) or 0)
				local v=0
				if     ups.get(a.."0") then v=-32767 if(o>0) then o=0 end
				elseif ups.get(a.."1") then v= 32767 if(o<0) then o=0 end
				end
				if v==0 then
					v=math.floor(o*0.8+v*0.2)
				else
					v=math.floor(o*0.95+v*0.05)
				end
				ups.set_axis({[ak]=v})
			end
			
			-- pick best axis l/r axis
			local lx ,ly ,lz =(ups.get("lx")  or 0),(ups.get("ly")  or 0),(ups.get("lz")  or 0)
			local lxk,lyk,lzk=(ups.get("lxk") or 0),(ups.get("lyk") or 0),(ups.get("lzk") or 0)
			if lxk*lxk+lyk*lyk+lzk*lzk > lx*lx+ly*ly+lz*lz then		ups.set_axis({lxb=lxk,lyb=lyk,lzb=lzk})
			else													ups.set_axis({lxb=lx ,lyb=ly ,lzb=lz })
			end
			local rx ,ry ,rz =(ups.get("rx")  or 0),(ups.get("ry")  or 0),(ups.get("rz")  or 0)
			local rxk,ryk,rzk=(ups.get("rxk") or 0),(ups.get("ryk") or 0),(ups.get("rzk") or 0)
			if rxk*rxk+ryk*ryk+rzk*rzk > rx*rx+ry*ry+rz*rz then		ups.set_axis({rxb=rxk,ryb=ryk,rzb=rzk})
			else													ups.set_axis({rxb=rx ,ryb=ry ,rzb=rz })
			end
		end

		return key
	end


-- turn a joystick msg into a key name or nil
-- this works on all axis inputs (bigest movement is chosen)
	function ups.joystick_msg_to_key(m)
		if m.class=="joystick" then
--print(m.lx,m.ly)
			local d=3/8
			if  		m.ly and m.ly>d 	then
				if    	m.lx and m.lx>d		then	return "down_right"
				elseif 	m.lx and m.lx<-d 	then	return "down_left"
				else								return "down"
				end
			elseif		m.ly and m.ly<-d 	then
				if     	m.lx and m.lx>d		then	return "up_right"
				elseif 	m.lx and m.lx<-d 	then	return "up_left"
				else								return "up"
				end
			else
				if     	m.lx and m.lx>d		then	return "right"
				elseif 	m.lx and m.lx<-d 	then	return "left"
				end
			end

		end
	end

-- as above but this works on given axis values, expected to be +-1 range
	function ups.joystick_axis_to_key(vx,vy)
		local d=1/4
		local vxx,vyy
		local nox,noy
		
		vxx=vx*vx				
		vyy=vy*vy				

		if vxx/2 > vyy then noy=true end
		if vyy/2 > vxx then nox=true end
		
		if not nox then
			if     vx>d		then	return "right"
			elseif vx<-d 	then	return "left"
			end
		end

		if not noy then
			if    	vy>d 	then	return "down"
			elseif	vy<-d 	then	return "up"
			end
		end
	end

	ups.setup(1)
]]

	ups.setkeymap("full")
	return ups
end

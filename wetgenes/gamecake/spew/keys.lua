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
	
	local recaps=oven.rebake("wetgenes.gamecake.spew.recaps")
	
	keys.defaults={}
-- single player covering entire keyboard (also merge the island1&2 keys in here)
	keys.defaults["full"]={
		["1"]			=	"select",
		["2"]			=	"start",
		["-"]			=	"l1",
		["="]			=	"r1",
		["["]			=	"l2",
		["]"]			=	"r2",
	}
-- 1up/2up key islands
	keys.defaults["island1"]={
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["."]			=	{"fire","x"},
		["/"]			=	{"fire","x"},
		["shift_r"]		=	{"fire","y"},
		["alt_r"]		=	{"fire","a"},
		["control_r"]	=	{"fire","b"},
		["space"]		=	{"fire","x"},
		["return"]		=	{"fire","a"},
		["enter"]		=	{"fire","b"},
	}
	for n,v in pairs(keys.defaults["island1"]) do keys.defaults["full"][n]=v end

	keys.defaults["island2"]={
		["w"]			=	"up",
		["s"]			=	"down",
		["a"]			=	"left",
		["d"]			=	"right",
		["<"]			=	{"fire","y"},
		["z"]			=	{"fire","y"},
		["shift_l"]		=	{"fire","x"},
		["control_l"]	=	{"fire","a"},
		["alt_l"]		=	{"fire","b"},
		["shift"]		=	{"fire","x"}, -- 1up grabs both the shift
		["control"]		=	{"fire","a"}, -- control and alt keys
		["alt"]			=	{"fire","b"}, -- if we cant tell left from right
	}
	for n,v in pairs(keys.defaults["island2"]) do keys.defaults["full"][n]=v end

-- single player mame/picade style buttons
	keys.defaults["pimoroni"]={
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["5"]			=	"select",
		["return"]		=	"start",
		["enter"]		=	"start",
		["shift_l"]		=	{"fire","a"},
		["z"]			=	{"fire","b"},
		["x"]			=	"l1",
		["control_l"]	=	{"fire","x"},
		["alt_l"]		=	{"fire","y"},
		["space"]		=	"r1",
		["1"]			=	"l2",
		["esc"]			=	"r2",
	}

	function keys.setup(opts)
		if type(opts)=="number" then opts={max_up=opts} end
		if not opts then opts={} end
		keys.opts=opts
		recaps.setup(opts)	-- also setup recaps

		opts.max_up=opts.max_up or 1
		keys.up={}
		for i=1,opts.max_up do
			keys.up[i]=keys.create(i,opts) -- 1up 2up etc
		end
		
		if opts.max_up==1 then -- single player, grab lots of keys
			if oven.opts.bake and oven.opts.bake.smell=="pimoroni" then
				keys.opts.notyping=true
				for n,v in pairs(keys.defaults["pimoroni"]) do
					keys.up[1].set(n,v)
				end
			else
				for n,v in pairs(keys.defaults["full"]) do
					keys.up[1].set(n,v)
				end
			end
		else
			for i=1,opts.max_up do -- multiplayer use keyislands so we can all fit on a keyboard
				for n,v in pairs(keys.defaults["island"..i] or {}) do
					keys.up[i].set(n,v)
				end
			end
		end
				
		return keys -- so setup is chainable with a bake
	end

	function keys.set_opts(n,v)
		if n=="typing" and keys.opts.notyping then v=false end -- disable typing../
		if n=="typing" and keys.opts.yestyping then v=true end -- disable non typing../
		keys.opts[n]=v
	end

-- convert keys or whatever into recaps changes
	function keys.msg(m)
		if not keys.up then return end -- no key maping

		local used=false
		for i,v in ipairs(keys.up) do -- possibly sort the joy msgs here...
--			if m.class=="posix_joystick" and ( m.code==59 or m.code==60 or m.code==61 ) then -- ignore crappy acc spam from a ds3
--			else
--print(wstr.dump(m))
				local t=v.msg(m)
				used=used or t
--			end
		end
		return used
	end
	

-- a players key mappings, maybe we need multiple people on the same keyboard or device
	function keys.create(idx,opts)
		local key={}
		key.opts=opts or {}
		key.idx=idx
		key.maps={}
		
		key.joy={}					
		key.joy.class="joystick"
		key.joy.lt=0
		key.joy.rt=0
		key.joy.lx=0
		key.joy.rx=0
		key.joy.dx=0
		key.joy.ly=0
		key.joy.ry=0
		key.joy.dy=0
		
		key.posxbox=true -- assume xbox

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
--print(key.last_joydir,joydir)
	--print(wstr.dump(m))
						local setclr={}
						if key.last_joydir then -- clear all old keys
							for _,n in ipairs(map[key.last_joydir]  ) do setclr[n]="clr" end
							used=true
						end
						key.last_joydir=joydir
						if joydir then -- set all new keys
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
--print( m.keyname)
--print(m.keyname)
					for n,v in pairs(key.maps) do
						if m.keyname==n or m.ascii==n then
							if m.action==1 then -- key set
--print("->"..table.concat( (type(v)=="table") and v or {v},",") )
								ups.set_button(v,true)
								used=true
							elseif m.action==-1 then -- key clear
								ups.set_button(v,false)
								used=true
							end
						end
					end
				end

			elseif m.class=="mouse" then -- swipe to move

				ups.set_axis({mx=m.x,my=m.y}) -- tell recap about the mouse positions, mx,my

				if m.action==1 then -- key set
					if m.keyname then ups.set_button("mouse_"..m.keyname,true) end
					if m.keyname=="left" or m.keyname=="right" or m.keyname=="middle" then
						ups.set_button("fire",true)
					end
					used=true
				elseif m.action==-1 then -- key clear
					if m.keyname then ups.set_button("mouse_"..m.keyname,false) end
					if m.keyname=="left" or m.keyname=="right" or m.keyname=="middle" then
						ups.set_button("fire",false)
					end
					used=true
				end

-- swipe to keypress code
-- this can be a problem in menus, so is best to only turn it on during gameplay

				if key.opts.swipe then -- use touch/mouse to swipe
				
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
			
			elseif m.class=="posix_joystick" then
				if key.idx-1==m.posix_num%key.opts.max_up then -- only take inputs from one joystick for multiplayer
					if m.type==1 then -- keys

						local docode=function(name)
							if m.value==1 then -- key set
								ups.set_button(name,true)
								used=true
							elseif m.value==0 then -- key clear
								ups.set_button(name,false)
								used=true
							end
						end

-- check for xbox key codes
						if     m.code==704 then docode("left")
						elseif m.code==705 then docode("right")
						elseif m.code==706 then docode("up")
						elseif m.code==707 then docode("down")
						elseif m.code==304 then docode("a") docode("fire")
						elseif m.code==305 then docode("b") docode("fire")
						elseif m.code==307 then docode("x") docode("fire")
						elseif m.code==308 then docode("y") docode("fire")
						elseif m.code==310 then docode("l1")
						elseif m.code==311 then docode("r1")
						elseif m.code==314 then docode("select")
						elseif m.code==315 then docode("start")
-- check for ps3 key codes
						elseif m.code==292 then docode("up")
						elseif m.code==293 then docode("right")
						elseif m.code==294 then docode("down")
						elseif m.code==295 then docode("left")
						elseif m.code==296 then docode("l2")
						elseif m.code==297 then docode("r2")
						elseif m.code==298 then docode("l1")
						elseif m.code==299 then docode("r1")
						elseif m.code==300 then docode("y") docode("fire")
						elseif m.code==301 then docode("b") docode("fire")
						elseif m.code==302 then docode("a") docode("fire")
						elseif m.code==303 then docode("x") docode("fire")
						else
--print(wstr.dump(m))
							docode("fire") -- all other buttons are fire
						end

					elseif m.type==3 then -- sticks ( assume ps3/xbox config )


						local docode=function(name)
							if m.value==0 then -- key clear
								ups.set_button(name,false)
								used=true
							else -- key set
								ups.set_button(name,true)
								used=true
							end
						end

						local fixy=function(v)
							if v<0 then key.posxbox=true end
							if not key.posxbox then v=v*2-1 end -- ps3 range is 0,+1 xbox is -1,+1
							return v
						end
						
						local active=false
						if m.code==59 or m.code==60 or m.code==61 then
							key.posxbox=false
-- stupid ps3 controller waving around bullshit
						elseif m.code==0 then
							key.joy.lx=fixy(m.value)
							active=true
							used=true
						elseif m.code==1 then
							key.joy.ly=fixy(m.value)
							active=true
							used=true
						elseif m.code==2 then
							key.joy.lt=m.value
							active=true
							used=true
						elseif m.code==5 then
							key.joy.rt=m.value
							active=true
							used=true
						elseif m.code==16 then
							key.joy.dx=m.value
							active=true
							used=true
						elseif m.code==17 then
							key.joy.dy=m.value
							active=true
							used=true
						elseif m.code== 44 then docode("up")				key.posxbox=false
						elseif m.code== 45 then docode("right")				key.posxbox=false
						elseif m.code== 46 then docode("down")				key.posxbox=false
						-- 47? left seems to be missing?
						elseif m.code== 48 then docode("l2")				key.posxbox=false
						elseif m.code== 49 then docode("r2")				key.posxbox=false
						elseif m.code== 50 then docode("l1")				key.posxbox=false
						elseif m.code== 51 then docode("r1")				key.posxbox=false
						elseif m.code== 52 then docode("y") docode("fire")	key.posxbox=false
						elseif m.code== 53 then docode("b") docode("fire")	key.posxbox=false
						elseif m.code== 54 then docode("a") docode("fire")	key.posxbox=false
						elseif m.code== 55 then docode("x") docode("fire")	key.posxbox=false
						else
--print(wstr.dump(m))
						end

						if active then
							new_joydir( keys.joystick_msg_to_key(key.joy) )
							ups.set_axis(key.joy) -- tell recap about the joy positions
						end
					end
				end
			elseif m.class=="joystick" then

				new_joydir( keys.joystick_msg_to_key(m) )
				ups.set_axis(m) -- tell recap about the joy positions

			elseif m.class=="joykey" then



				if m.action==1 then -- key set
					ups.set_button("fire",true)
					used=true
				elseif m.action==-1 then -- key clear
					ups.set_button("fire",false)
					used=true
				end

			end
			
			return used -- if we used the msg
		end
				
		return key
	end


-- turn a joystick msg into a key name or nil
-- this works on all axis inputs (bigest movement is chosen)
	function keys.joystick_msg_to_key(m)
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
	function keys.joystick_axis_to_key(vx,vy)
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

	return keys
end

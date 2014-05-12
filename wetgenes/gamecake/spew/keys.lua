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
-- single player covering entire keyboard
	keys.defaults["full"]={
		["w"]			=	"up",
		["s"]			=	"down",
		["a"]			=	"left",
		["d"]			=	"right",
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["1"]			=	"select",
		["2"]			=	"start",
		["return"]		=	"fire",
		["enter"]		=	"fire",
		["shift_l"]		=	{"fire","x"},
		["<"]			=	{"fire","y"},
		["z"]			=	{"fire","y"},
		["."]			=	{"fire","x"},
		["/"]			=	{"fire","x"},
		["shift_r"]		=	{"fire","y"},
		["control_l"]	=	{"fire","a"},
		["alt_l"]		=	{"fire","b"},
		["space"]		=	"fire",
		["alt_r"]		=	{"fire","x"},
		["control_r"]	=	{"fire","y"},
		["-"]			=	"l1",
		["["]			=	"l2",
		["="]			=	"r1",
		["]"]			=	"r2",
	}
-- 1up/2up key islands
	keys.defaults["island1"]={
		["w"]			=	"up",
		["s"]			=	"down",
		["a"]			=	"left",
		["d"]			=	"right",
		["shift_l"]		=	"fire",
		["control_l"]	=	"fire",
		["alt_l"]		=	"fire",
	}
	keys.defaults["island2"]={
		["up"]			=	"up",
		["down"]		=	"down",
		["left"]		=	"left",
		["right"]		=	"right",
		["shift_r"]		=	"fire",
		["control_r"]	=	"fire",
		["alt_r"]		=	"fire",
	}
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
		keys.opts[n]=v
	end

-- convert keys or whatever into recaps changes
	function keys.msg(m)
		if not keys.up then return end -- no key maping

		local used=false
		for i,v in ipairs(keys.up) do -- possibly sort the joy msgs here...
			local t=v.msg(m)
			used=used or t
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
			local ups=recaps.ups(key.idx)
--			if not ups then return end -- nowhere to send the data
			
			local new_joydir=function(joydir)
					-- this does not handle diagonal movement, forces one of 4 directions.
					if key.last_joydir~=joydir then -- only when we change
	--print(wstr.dump(m))
						if key.last_joydir then -- first clear any previous key
							ups.set_button(key.last_joydir,false)
							used=true
						end
						key.last_joydir=joydir
						if joydir then
							ups.set_button(joydir,true) -- then send any new key
							used=true
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

			elseif m.class=="mouse" then -- swipe to move

				ups.set_axis({mx=m.x,my=m.y}) -- tell recap about the mouse positions, mx,my

				if m.action==1 then -- key set
					if m.keyname then ups.set_button("mouse_"..m.keyname,true) end
					ups.set_button("fire",true)
					used=true
				elseif m.action==-1 then -- key clear
					if m.keyname then ups.set_button("mouse_"..m.keyname,false) end
					ups.set_button("fire",false)
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

				if m.type==1 then -- keys

					if m.value==1 then -- key set
						ups.set_button("fire",true)
						used=true
					elseif m.value==0 then -- key clear
						ups.set_button("fire",false)
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
						new_joydir( keys.joystick_msg_to_key(key.joy) )
						ups.set_axis(keys.joy) -- tell recap about the joy positions
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
			local d=1/8
			local t,vx,vy
			local tt,vxx,vyy
			local nox,noy

			vx=m.lx		vxx=m.lx*m.lx				
			t=m.rx		tt=t*t			if tt>vxx then vx=t vxx=tt end
			t=m.dx		tt=t*t			if tt>vxx then vx=t vxx=tt end

			vy=m.ly		vyy=m.ly*m.ly				
			t=m.ry		tt=t*t			if tt>vyy then vy=t vyy=tt end
			t=m.dy		tt=t*t			if tt>vyy then vy=t vyy=tt end
		
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
	end

-- as above but this works on given axis values, expected to be +-1 range
	function keys.joystick_axis_to_key(vx,vy)
		local d=1/8
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

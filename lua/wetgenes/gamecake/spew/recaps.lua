--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,recaps)

	recaps=recaps or {} 

	recaps.ups=oven.ups.up

	return recaps
end


-- remove all old code and fake it
--[==[

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


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



M.bake=function(oven,recaps)

	recaps=recaps or {} 
	
	local cake=oven.cake
	local canvas=cake.canvas
	
--	local keys=oven.rebake("wetgenes.gamecake.spew.keys")

	function recaps.setup(opts)
--print("recaos",opts)	
		if type(opts)=="number" then opts={max_up=opts} end
		if not opts then opts={} end
		recaps.opts=opts

		opts.max_up=opts.max_up or 1
		recaps.up={}
		for i=1,opts.max_up do
			recaps.up[i]=recaps.create(i) -- 1up 2up etc
		end
		return recaps -- so setup is chainable with a bake
	end

	function recaps.push()
		local m={}
		m.ups={}
		for i,v in ipairs(recaps.up or {}) do
			m.ups[i]=v.get_change() or {}
			v.step()
		end
		oven.tasks.linda:send(nil,"ups",m)
--[[
		local up=recaps.up and recaps.up[1]
		for i,v in ipairs(recaps.up or {}) do
			v.step()
		end
		local m={}
		m.msgs=up and up.state_msgs
		m.ups={}
		for i,v in ipairs(recaps.up or {}) do
			local u={}
			m.ups[i]=u
			u.idx=i
			u.state=v.state
			u.state_pulse=v.state_pulse
		end
		oven.tasks.linda:send(nil,"ups",m)
--print("up","push",m)
]]
	end
	
	function recaps.pull()
		local up=recaps.up and recaps.up[1]
		if up then up.state_msgs={} end -- clr old msgs
		for i,v in ipairs(recaps.up or {}) do
			v.state_pulse={} -- clr old pulses
		end

		repeat
			local ok,m=oven.tasks.linda:receive(0,"ups") -- grab all available memos
			if ok and m then
				for i,v in ipairs(m.ups or {}) do
					local up=recaps.up[i]
					up.step_apply(v)
				end
			end
		until not m

--[[
		local up=recaps.up and recaps.up[1]
		up.state_msgs={} -- clr old msgs
		for i,v in ipairs(recaps.up or {}) do
			v.state_pulse={} -- clr old pulses
		end

		repeat
			local ok,m=oven.tasks.linda:receive(0,"ups") -- grab all available memos
			if ok and m then
--print("up","pull",m)
				if up then
					local l=#up.state_msgs
					for i=1,#m.msgs do -- append
						up.state_msgs[l+i]=m.msgs[i]
					end
				end
				for i,v in ipairs(recaps.up or {}) do
					local u=m.ups[i]
					if u then
						for a,b in pairs(u.state) do v.state[a]=b end -- merge state with old state
						for a,b in pairs(u.state_pulse) do v.state_pulse[a]=b end -- merge pulse with other recent pulses
					end
				end
			end
		until not m
]]
	end

	function recaps.step()
		if oven.is.update then -- pull msgs from other thread

			recaps.pull()

		elseif oven.is.main then -- push msgs to other thread

			recaps.push()

		else -- only one thread

			for i,v in ipairs(recaps.up or {}) do
				v.step()
			end

		end
	end
	
-- pick one of the up[idx] tables
	function recaps.ups(idx)
		if idx==0 then -- merge all buttons and axis of all controllers
			local up={}
			
			up.button=function(name)
				local b=false
				for i=1,#recaps.up do
					b=b or recaps.up[i].button(name)
				end
				return b
			end

			up.axis=function(name)
				local n=0
				local t=0
				for i=1,#recaps.up do
					local v=recaps.up[i].axis(name)
					if v then
						n=n+v
						t=t+1
					end
				end
				if t>0 then return math.floor(n/t) end
			end
			
			up.axisfixed=function(name)
				local t=up.axis(name)
				if t then return fixaxis(t) end
				return 0
			end

			up.msgs=function(name)
				return recaps.up[1].msgs(name)
			end

			return up
		end
		return recaps.up and ( recaps.up[idx or 1] or recaps.up[1] )
	end

-- create a new recap table, then we can load or save this data to or from our server
	function recaps.create(idx)
		local recap={}
		recap.idx=idx
		
-- these 4 functions are the only ones a user needs
-- all others are internal
		recap.button=function(name) -- return state "valid" frame data not current "volatile" frame data
			if name then
				return recap.state[name] or recap.state_pulse[name]
			end
			return recap.state -- return all buttons if no name given but this does not include pulses	
		end

		recap.axis=function(name) -- return state "valid" frame data not current "volatile" frame data
			if name then
				return recap.state[name]
			end
			return recap.state -- return all axis if no name given
		end

		recap.axisfixed=function(name) -- fix deadzones etc
			local ax=recap.axis(name)
			if ax then return fixaxis( ax ) end
			return 0
		end
-- see above


		function recap.reset(flow)
			recap.flow=flow or "none" -- do not play or record by default

			recap.state_msgs={} -- list of msgs for this tick
			recap.now_msgs={} -- live list that gets swapped into state
--			recap.now_qualifiers={} -- flags of "alt" , "ctrl" , "shift" , "win"  in 1,2,3,4 so we know which keys are held down
--			recap.now_qualifiers_text=nil -- string of the above flags joined by + or nil
			
			recap.state={}
			recap.now={}
			
			recap.state_pulse={}
			recap.now_pulse={}

			recap.stream={} -- a stream of change "table"s or "number" frame skips
			recap.frame=0
			recap.read=0
			recap.wait=0
			recap.touch="" -- you can replace this with a requested touch control scheme

-- "left_right" is a two button touch screen split
			
		end
		
--[[
		recap.build_qualifiers_text=function()
			local s
			local add=function(key) if key then if s then s=s.."+"..key else s=key end end end
			add(recap.now_qualifiers[1])
			add(recap.now_qualifiers[2])
			add(recap.now_qualifiers[3])
			add(recap.now_qualifiers[4])
			recap.now_qualifiers_text=s
		end
]]
		
		function recap.set(nam,dat) -- set the volatile data,this gets copied into state before it should be used
			recap.now[nam]=dat
		end
		function recap.get(nam) -- get the volatile data
			local l=recap.now[nam] -- now only contains most recent changes
			if type(l)=="nil" then l=recap.state[nam] end -- so we also need state
			return l
		end
		function recap.pulse(nam,dat) -- set the volatile data but *only* for one frame
			recap.now_pulse[nam]=dat
		end
		


		recap.msgs=function(class) -- list of msgs for the last tick with optional simple class filter
			if class then
				return recap.state_msgs[class] or {}
			end
			return recap.state_msgs or {}
		end
		
-- use this to copy and remember a msg
		function recap.set_msg(mm)
			local m={}
			for n,v in pairs(mm) do m[n]=v end -- copy top level only
			recap.now_msgs[#recap.now_msgs+1]=m -- remember in main list
			if m.class then -- msg will have a class but just in case
				local list=recap.now_msgs[ m.class ]
				if not list then list={} ; recap.now_msgs[ m.class ]=list end -- start new class list
				list[ #list+1 ]=m -- also remember in class list
			end
--[[
			if m.class=="key" then
				if     ( m.keyname=="alt" or m.keyname=="alt_l" or m.keyname=="alt_r" ) then
					if     m.action== 1 then recap.now_qualifiers[1]="alt"
					elseif m.action==-1 then recap.now_qualifiers[1]=false end
					recap.build_qualifiers_text()
				elseif ( m.keyname=="control" or m.keyname=="control_l" or m.keyname=="control_r" ) then
					if     m.action== 1 then recap.now_qualifiers[2]="ctrl"
					elseif m.action==-1 then recap.now_qualifiers[2]=false end
					recap.build_qualifiers_text()
				elseif ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) then
					if     m.action== 1 then recap.now_qualifiers[3]="shift"
					elseif m.action==-1 then recap.now_qualifiers[3]=false end
					recap.build_qualifiers_text()
				elseif ( m.keyname=="gui" or m.keyname=="left gui" or m.keyname=="right gui" ) then -- gui/command/windows key?
					if     m.action== 1 then recap.now_qualifiers[4]="win"
					elseif m.action==-1 then recap.now_qualifiers[4]=false end
					recap.build_qualifiers_text()
				end
				m.qualifiers=recap.now_qualifiers_text -- add our cached qualifiers to key messages
			end
]]
		end

-- use this to set a joysticks axis position
		function recap.set_axis(m)
			for n,v in pairs(m) do
				recap.now[n]=v
			end
		end
-- use this to set a mouse axis relative movement
		function recap.set_axis_relative(m)
			for n,v in pairs(m) do
				recap.now[n]=((recap.state[n] or 0)+v)%65536
			end
		end


-- use this to set button flags, that may trigger a set/clr extra pulse state
		function recap.set_button(nam,v)
--print(nam,v)
			if type(nam)=="table" then
				if type(nam[2])=="number" then -- axis
					if v then
						recap.set_axis({[ nam[1] ]=nam[2]})	-- set
					else
						recap.set_axis({[ nam[1] ]=0}) -- clr
					end
				else
					for _,n in ipairs(nam) do recap.set_button(n,v) end -- multi
				end
			else
				local l=recap.get(nam)
				if v then -- set
					if not l then -- change?
						recap.set(nam,true)
						recap.pulse(nam.."_set",true)
					end
				else -- clr
					if l then -- change?
						recap.set(nam,false)
						recap.pulse(nam.."_clr",true)
					end
				end
			end
		end

-- get full current state as changeset
		function recap.get_state()
			local change={}
			for n,v in pairs(recap.state) do
				change[n]=v
			end
			for n,v in pairs(recap.pulse) do
				change[n]=v
			end
			change.msgs={}
			if #recap.now_msgs>0 then
				for i,v in ipairs(recap.msgs) do
					change.msgs[i]=v
				end
			end
			return change
		end

-- get diff of now state as changeset
		function recap.get_change()
			local change={}
			local changed=false

			for n,v in pairs(recap.now_pulse) do -- all of these represent a change
				changed=true
				change[n]=v
			end
			
			if #recap.now_msgs>0 then -- all msgs need to be kept as is
				changed=true
				change.msgs={}
				for i,v in ipairs(recap.now_msgs) do
					change.msgs[i]=v
				end
			end

			for n,v in pairs(recap.now) do
				if recap.state[n]~=v then -- a change
					changed=true
					change[n]=v
				end
			end
		
			return changed and change
		end

		function recap.step_next()

			for n,v in pairs(recap.now) do
				recap.state[n]=v
				recap.now[n]=nil
			end

			recap.state_pulse=recap.now_pulse
			recap.now_pulse={} -- new table
			
			recap.state_msgs=recap.now_msgs -- use this copy of any messages
			recap.now_msgs={} -- start a new list

		end
		
		function recap.step_apply(change)
			recap.state_pulse={}
			recap.state_msgs={}
			for n,v in pairs(change) do
				local tail=string.sub(n,-4)
				if tail=="_set" or tail=="_clr" then
					recap.state_pulse[n]=v
				elseif n=="msgs" then
					for _,m in ipairs(v) do
						recap.state_msgs[ #recap.state_msgs+1 ]=m
						if m.class then -- also remember in class sub list
							if not recap.state_msgs[m.class] then recap.state_msgs[m.class]={} end -- new
							local list=recap.state_msgs[m.class]
							list[ #list+1 ]=m -- append
						end
					end
				else
					recap.state[n]=v
				end
			end
		end

		function recap.step(flow)
			flow=flow or recap.flow

--print("step "..tostring(flow))	

			if flow=="record" then

				local change=recap.get_change()
				recap.step_next()

				if change then
					table.insert(recap.stream,change) -- change something
				else
					if type(recap.stream[#recap.stream])=="number" then
						recap.stream[#recap.stream] = recap.stream[#recap.stream] + 1 -- keep on changing nothing
					else
						table.insert(recap.stream,1) -- change nothing
					end
				end
			
			elseif flow=="play" then -- grab from the stream
			
				recap.state_pulse={} -- always empty the pulses
				recap.state_msgs={} -- and msgs

				if recap.wait>0 then
				
					recap.wait=recap.wait-1
					
				else
				
					recap.read=recap.read+1
					
					local t=recap.stream[recap.read]
					local tt=type(t)
					
					if tt=="number" then
						recap.wait=t-1
					elseif tt=="table"then
						recap.step_apply(t)
					end
				end
			
			else -- default of do not record, do not play just be
			
--				local change=recap.get_change()
--				if change then change.idx=recap.idx ; dump(change) end

				recap.step_next()

--				local change=recap.get_state()
--				if change then change.idx=recap.idx ; dump(change) end

			end
			
			recap.frame=recap.frame+1 -- advance frame counter
		end
		
		recap.reset()

		return recap
	end


	recaps.setup(1)
	return recaps
end

]==]

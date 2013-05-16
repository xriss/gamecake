-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")

-- handle a simple state for win programs,
-- all it does is call other states/mods functions.

local function print(...) return _G.print(...) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(opts)

	local state={}

		state.opts=opts
		opts.state=state
		
		state.baked={}
		state.mods={}

-- you may perform this bake_cake yourself if you want more control in an app
		
		function state.bake_cake()

			state.frame_rate=1/opts.fps -- how fast we want to run
			state.frame_time=0

			local inf={width=opts.width,height=opts.height,title=opts.title}
			local screen=wwin.screen()

			inf.x=(screen.width-inf.width)/2
			inf.y=(screen.height-inf.height)/2

			state.win=wwin.create(inf)
			state.win:context({})

			state.rebake("wetgenes.gamecake") -- bake the cake into the state,

			-- the order these are added is important for priority, top of list is lowest priority, bottom is highest.
			state.require_mod("wetgenes.gamecake.mods.escmenu") -- escmenu gives us a doom style escape menu
			state.require_mod("wetgenes.gamecake.mods.console") -- console gives us a quake style tilda console
			state.require_mod("wetgenes.gamecake.mods.keys") -- touchscreen keys and posix keymaping
			state.require_mod("wetgenes.gamecake.mods.mouse") -- auto fake mouse on non window builds
			state.require_mod("wetgenes.gamecake.mods.layout") -- screen layout options

			if opts.state_next then
				state.next=state.rebake(opts.state_next)
			end
			
			return state
		end
		


-- require and bake state.baked[modules] in such a way that it can have simple circular dependencies

		function state.rebake(name)

			local ret=state.baked[name]
			
			if not ret then
			
				ret={modname=name}
				state.baked[name]=ret
				ret=assert(require(name)).bake(state,ret)
				
			end

			return ret
		end


-- this performs a rebake and adds the baked module into every update/draw function
-- so we may insert extra functionality without having to modify the running app
-- eg a console or an onscreen keyboard
		function state.require_mod(mname)
		
			local m=mname -- can pass in mod table or require name of mod
			if type(m)=="table" then mname=m.modname end
			if state.mods[mname] then return state.mods[mname] end -- already setup, nothing else to do
			if type(m)=="string" then m=state.rebake(mname) end -- rebake mod into this state

			state.mods[mname]=m			-- store baked version by its name
			table.insert(state.mods,m)		-- and put it at the end of the list for easy iteration
			
			m.setup() -- and call setup since it will always be running from now on until it is removed
			
			return m
		end
		

		if opts.times then
			state.times={}
			function state.times.create()
				local t={}
				t.time=0
				t.time_live=0
				
				t.hash=0
				t.hash_live=0
				
				t.started=0
				
				function t.start()
					t.started=state.win and state.win.time() or 0
				end
				
				function t.stop()
					local ended=state.win and state.win.time() or 0
					
					t.time_live=t.time_live + ended-t.started
					t.hash_live=t.hash_live + 1
				end
				
				function t.done()
					t.time=t.time_live
					t.hash=t.hash_live
					t.time_live=0
					t.hash_live=0
					
				end
				
				return t
			end
			state.times.update=state.times.create()
			state.times.draw=state.times.create()
		end

		function state.change()

		-- handle state changes

			if state.next then
			
				if state.now and state.now.clean then
					state.now.clean()
				end
				
				if type(state.next)=="string" then	 -- change by required name
				
					state.next=state.rebake(state.next)
					
				elseif type(state.next)=="boolean" then -- special exit state
				
					if wwin.hardcore.task_to_back then -- on android there is no quit, only back
						wwin.hardcore.task_to_back()
						state.next=nil
					else
						return true
					end
					
				end

				if state.next then
					state.last=state.now
					state.now=state.next
					state.next=nil
					
					if state.now and state.now.setup then
						state.now.setup()
					end
				end
				
			end
			
		end

		function state.setup()	
			if state.now and state.now.setup then
				state.now.setup()
			end
		end

		function state.start()	
			state.win:start()
			state.cake.start()
			if state.canvas then
				state.canvas.start()
			end
			if state.now and state.now.start then
				state.now.start()
			end
		end

		function state.stop()
			state.win:stop()
			state.cake.stop()
			if state.canvas then
				state.canvas.stop()
			end
			if state.now and state.now.stop then
				state.now.stop()
			end
		end

		function state.clean()
			if state.now and state.now.clean then
				state.now.clean()
			end
		end

		function state.update()

				if state.frame_rate and state.frame_time then --  framerate limiter enabled
					if state.frame_time<(state.win:time()-0.5) then state.frame_time=state.win:time() end -- prevent race condition
					while (state.frame_time)>state.win:time() do state.win:sleep(0.001) end -- simple frame limit
					state.frame_time=state.frame_time+state.frame_rate -- step frame forward one tick
				end

--print( "UPDATE",math.floor(10000000+(state.win:time()*1000)%1000000) )

				if state.times then state.times.update.start() end
				
				if state.now and state.now.update then
					state.now.update()
				end
				for i,v in ipairs(state.mods) do
					if v.update then
						v.update()
					end
				end

				if state.times then state.times.update.stop() end
				
			if state.frame_rate and state.frame_time then --  framerate limiter enabled
				if (state.frame_time-0.001)<=state.win:time() then -- repeat until we are ahead of real time
					return state.update() -- tailcall
				end
			end
		end

		function state.draw()
		
			if state.canvas then -- use each vb only once per frame
				state.canvas.draw()
			end
			
--print( "DRAW",math.floor(10000000+(state.win:time()*1000)%1000000) )

			if state.times then state.times.draw.start() end -- between calls to draw
			
			if state.now and state.now.draw then
				state.now.draw()
			end
			
			for i,v in ipairs(state.mods) do
				if v.draw then
					v.draw()
				end
			end
						
			if state.times then state.times.draw.stop() end -- draw is squify so just use it as the total time

			if state.win then
				state.win:swap()
			end
			
		end

		function state.msgs() -- read and process any msgs we have from win:msg
			if state.win then
				for m in state.win:msgs() do

					if m.class=="mouse" and m.x and m.y then	-- need to fix x,y numbers
						m.xraw,m.yraw=m.x,m.y					-- remember original
					end
					
					if m.class=="app" then -- androidy
--print("caught : ",m.class,m.cmd)
						if		m.cmd=="init_window" then
							state.start()
							state.paused=false
						elseif	m.cmd=="lost_focus"  then
							state.paused=true
						elseif	m.cmd=="gained_focus"  then
							state.paused=false
						elseif	m.cmd=="term_window"  then
							state.paused=true
							state.stop()
						end
					end

					for i=#state.mods,1,-1 do -- run it through the mods backwards, so the topmost layer gets first crack at the msgs
						local v=state.mods[i]
						if m and v and v.msg then
							m=v.msg(m) -- mods can choose to eat the msgs, they must return it for it to bubble down
						end
					end
					if m and state.now and state.now.msg then
						state.now.msg(m)
					end
				end
			end
		end

-- a busy blocking loop, or not, if we are running on the wrong sort
-- of system it just returns and expects the other functions
-- eg state.serv_pulse to be called when necesary.
		function state.serv(state)
		
			if state.win.noblock then
				return state
			end
			
			local finished
			repeat
				finished=state.serv_pulse(state)
			until finished
		end
		function state.serv_pulse(state)
				if state.change() then return true end
				state.msgs()
				
				state.cake.update()
				if not state.paused then
					state.update()
					state.draw()
				else
					state.win:sleep(1/10)
				end
				
--				return state.change()
		end
		

	return state

end

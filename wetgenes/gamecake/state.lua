-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")

-- handle a simple state for win programs,
-- all it does is call other states/mods functions.

local function print(...) return _G.print(...) end

module(...)

function bake(opts)

	local state={}

		state.opts=opts
		state.baked={}
		state.mods={}
		
-- require and bake state.baked[modules] in such a way that it can have simple circular dependencies

		function state.rebake(state,name)

			local ret=state.baked[name]
			
			if not ret then
			
				ret={modname=name}
				state.baked[name]=ret
				ret=assert(require(name)).bake(state,ret)
				
			end

			return ret
		end


-- this should be replaced with the above rebake function
		function state.require_mod(mname)
		
			local m=mname -- can pass in mod table or require name of mod
			if type(m)=="string" then m=state:rebake(m) end

			if state.mods[m.name] then return state.mods[m.name] end -- already setup, nothing else to dore

			state.mods[m.name]=m			-- store baked version by its name
			table.insert(state.mods,m)		-- and put it at the end of the list for easy iteration
			
			m.setup(state) -- and auto setup
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
					state.now.clean(state)
				end
				
				if type(state.next)=="string" then	 -- change by required name
				
					state.next=state:rebake(state.next)
					
				elseif type(state.next)=="boolean" then -- special exit state
				
					return true
					
				end

				state.last=state.now
				state.now=state.next
				state.next=nil
				
				if state.now and state.now.setup then
					state.now.setup(state)
				end
				
			end
			
		end

		function state.setup()	
			if state.now and state.now.setup then
				state.now.setup(state)
			end
		end

		function state.start()	
			state.win:start()
			state.cake.start()
			if state.now and state.now.start then
				state.now.start(state)
			end
		end

		function state.stop()
			state.win:stop()
			state.cake.stop()
			if state.now and state.now.stop then
				state.now.stop(state)
			end
		end

		function state.clean()
			if state.now and state.now.clean then
				state.now.clean(state)
			end
		end

		function state.update()

				if state.frame_rate and state.frame_time then --  framerate limiter enabled
					if state.frame_time<(state.win:time()-0.5) then state.frame_time=state.win:time() end -- prevent race condition
					while (state.frame_time-0.001)>state.win:time() do state.win:sleep(0.001) end -- simple frame limit
					state.frame_time=state.frame_time+state.frame_rate -- step frame forward one tick
				end

				if state.times then state.times.update.start() end
				
				if state.now and state.now.update then
					state.now.update(state)
				end
				for i,v in ipairs(state.mods) do
					if v.update then
						v.update(state)
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
			
			if state.times then state.times.draw.stop() end -- draw is squify so just use it as the total time
			if state.times then state.times.draw.start() end -- between calls to draw
			
			if state.now and state.now.draw then
				state.now.draw(state)
			end
			
			for i,v in ipairs(state.mods) do
				if v.draw then
					v.draw(state)
				end
			end
						
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
						elseif	m.cmd=="pause"  then
							state.paused=true
						elseif	m.cmd=="term_window"  then
							state.paused=true
							state.stop()
						end
					end

					if state.now and state.now.msg then
						state.now.msg(state,m)
					end
					for i,v in ipairs(state.mods) do
						if v.msg then
							v.msg(state,m)
						end
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
				state.msgs()
				
				state.cake.update()
				if not state.paused then
					state.update()
					state.draw()
				else
					state.win:sleep(1/10)
				end
				
				finished=state.change()
				return finished
		end
		

	return state

end

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- handle a simple state for win programs,
-- all it does is call other states/mods functions.

module(...)

function bake(opts)

	local state={}

		state.mods={}
		
		function state.require_mod(mname)
		
			local m=require(mname)
			
			if state.mods[m.name] then return state.mods[m.name] end -- already setup
			
			local mb=m.bake(opts) -- bake it
			
			state.mods[m.name]=mb			-- store baked version by its name
			table.insert(state.mods,mb)		-- and put it at the end of the list for easy iteration
			
			mb.setup(state) -- and setup
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
					if type(state.next)=="boolean" then -- special exit state
						return true
					end
					state.next=require(state.next)
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

		function state.clean()
			if state.now and state.now.clean then
				state.now.clean(state)
			end
		end

		function state.update()

			if state.frame_rate and state.frame_time then --  framerate limiter enabled
				if state.frame_time<state.win:time() then state.frame_time=state.win:time() end -- prevent race condition
				while (state.frame_time-0.001)>state.win:time() do state.win:sleep(0.001) end -- simple frame limit
				state.frame_time=state.frame_time+state.frame_rate -- step frame forward
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
		end

		function state.draw()
			
			if state.times then state.times.draw.stop() end -- draw is squify just use it as total frame
			if state.times then state.times.draw.start() end
			
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
				local m=state.win:msg() -- read first
				while m[1] do
					if state.now and state.now.msg then
						state.now.msg(m)
					end
					for i,v in ipairs(state.mods) do
						if v.msg then
							v.msg(m)
						end
					end
					m=state.win:msg() -- read next
				end
			end
		end

	return state

end

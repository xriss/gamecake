-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- handle a simple state for win programs,
-- all it does is call other states/mods functions.

module(...)

function bake(opts)

	local state={}

		state.mods={}

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
				while state.frame_time>state.win:time() do state.win:sleep(0.001) end -- simple frame limit
				state.frame_time=state.frame_time+state.frame_rate -- step frame forward
			end
			
			if state.now and state.now.update then
				state.now.update(state)
			end
			for i,v in ipairs(state.mods) do
				if v.update then
					v.update(state)
				end
			end
		end

		function state.draw()
			
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


	return state

end

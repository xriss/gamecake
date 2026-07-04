--
-- (C) 2024 Kriss@XIXs.com
--

-- module
local M={ modname = (...) } package.loaded[M.modname] = M

--[[#lua.wetgenes.gamecake.fun.bitsynth_task

	local bitsynth_task=require("wetgenes.gamecake.fun.bitsynth_task")

Manages synth rendering in a subtask, possibly multiple subtasks, so as 
not to block the main thread when generating synth sounds.

]]

------------------------------------------------------------------------
do -- be very careful not to leak locals into task function
------------------------------------------------------------------------

M.functions={}
M.metatable={__index=M.functions}
setmetatable(M,M.metatable)

--[[#lua.wetgenes.gamecake.fun.bitsynth_task.render


	local f=bitsynth_task.render(tasks,ot)

	-- busy check for result , dont do this :)
	repeat
		local dat,err=f()
		if dat then -- we loaded it
			-- use sound here
		else
			if err then error(err) end -- caught an error
		end
	until dat

returns a function that will peek for a return msg and return data (s16 
sound data string) on success.

on error it will return nil,error so need to check the second return if 
you want to handle errors.

after a successful result then subsequent calls will return nil,"done" 
indicating that the task has already completed.

]]
M.functions.render=function(tasks,ot)

	local done=false
	local memo={}
	memo.task="bitsynth"

	memo.cmd="bitsynth"
	memo.ot=ot -- bitsynth options table
	
	tasks:send(memo) -- send

	return function()

		if done then return nil,"done" end
		local got=tasks:receive(memo,0) -- peek

		if got then -- check we did not time out waiting
			done=true

			if memo.error then return nil,memo.error end
			if memo.result.error then return nil,memo.result.error end

			return memo.result.dat
		else
			return nil -- waiting
		end

	end
end
------------------------------------------------------------------------
end -- The functions below are free running tasks and should not depend on any locals
------------------------------------------------------------------------

--[[#lua.wetgenes.gamecake.fun.bitsynth_task.code

lanes task function for handling bitsynth rendering.

]]
M.functions.code=function(linda,task_id,task_idx)
	local M -- hide M for thread safety
	local global=require("global") -- lock accidental globals

	local task_id_msg=task_id..":synth"

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end
	
	local bitsynth=require("wetgenes.gamecake.fun.bitsynth")
	local fats=require("wetgenes.fats")

	local request=function(memo)
		local ret={}
		
		if memo.cmd=="bitsynth" then
			
			local it=bitsynth.prepare(memo.ot)
			local tt=bitsynth.render(it)
			local tab=bitsynth.float_to_16bit(tt)
			local dat=fats.table_to_int16s(tab)

			ret.dat=dat -- return 16bit data string from bitsynth
		end
	
		return ret
	end

	while true do

		local _,memo= linda:receive( 1 , task_id ) -- wait for any memos coming into this thread

		if memo then
			local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end

end

--
-- (C) 2025 Kriss@XIXs.com
--

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local all=require("wetgenes.gamecake.zone.system.all") -- put everything in here
-- sub tasks for running in other threads
all.code=all.code or {}
M.code=all.code

-- initalize systems in the scene
all.scene.start_all_tasks=function(scene)

	local oven=scene.oven

	-- create threads and init then	

	oven.tasks:add_global_thread({
		count=1,
		id="all_popins",
		code=all.code.popins,
	})

	oven.tasks:add_global_thread({
		count=1,
		id="all_values",
		code=all.code.values,
	})

	oven.tasks:add_global_thread({
		count=1,
		id="all_tweens",
		code=all.code.tweens,
	})


	oven.tasks:do_memo({
		task="all_values",
		id=false,
		cmd="scene",
		scene=scene.wrap_name,
		call="full_setup",
	})

	oven.tasks:do_memo({
		task="all_values",
		id=false,
		cmd="subscribe",
		subid="all_test",
	})

end


-- task that deals with popin/out and other world generation
all.code.popins=function(linda,task_id,task_idx)
	local M,all -- hide for thread safety
	local global=require("global") -- lock accidental globals

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end

	local main=function()
	end

	local request=function(memo)
		local ret={}
		return ret
	end

	while true do
		local timeout=0.001 -- first receive will be 1ms or less
		repeat
			local _,memo= linda:receive( timeout , task_id ) -- wait for any memos coming into this thread
			timeout=0 -- repeat receive are instant
			main() -- probably getting called every 1ms ish
			if memo then
				local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
				if not ok then ret={error=ret or true} end -- reformat errors
				if memo.id then -- result requested
					linda:send( nil , memo.id , ret )
				end
			end
		until not memo
	end

end

-- manage scene update and full syncs of values to other tasks
all.code.values=function(linda,task_id,task_idx)
	local M,all -- hide for thread safety
	local p=print
	print=function(...) return p(task_id,...) end
	local global=require("global") -- lock accidental globals

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end
	
	-- basic oven, nogl etc
	local oven=require("wetgenes.gamecake.toaster").bake({linda=linda})
	oven.upnet=oven.rebake("wetgenes.gamecake.upnet")

	local scene
	
	-- this should mostly not do anything as it will be called a lot
	local main=function()
		if scene then
			scene:do_update_values()
		end
	end

	local request=function(memo)
		local ret={}
		
		if memo.cmd=="scene" then
			if scene then
				scene:full_clean()
				scene=nil
			end
			if memo.scene then -- can delete scene by not naming one
				scene=require(memo.scene).create()
				scene.oven=oven
				scene.subscribed={}
				scene.infos.all.scene.initialize(scene)
				print("create",memo.scene)
				if memo.call then
					scene[memo.call](scene)
				end
			end
		elseif memo.cmd=="subscribe" then

			scene.subscribed[memo.subid]={}
			
			local values=scene:save_all_values(true) -- first full dump
			scene.oven.tasks:do_memo({
				task=memo.subid,
				id=false,
				cmd="values",
				values=values,
			})


		elseif memo.cmd=="unsubscribe" then

			scene.subscribed[memo.subid]=nil

		end
		
		return ret
	end

	while true do
		local timeout=0.001 -- first receive will be 1ms or less
		repeat
			local _,memo= linda:receive( timeout , task_id ) -- wait for any memos coming into this thread
			timeout=0 -- repeat receive are instant
			main() -- probably getting called every 1ms ish
			if memo then
				local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
				if not ok then ret={error=ret or true} end -- reformat errors
				if memo.id then -- result requested
					linda:send( nil , memo.id , ret )
				end
			end
		until not memo
	end

end

-- manage scene predictions and draw state generation ( draw needs to happen on main thread )
all.code.tweens=function(linda,task_id,task_idx)
	local M,all -- hide for thread safety
	local p=print
	print=function(...) return p(task_id,...) end
	local global=require("global") -- lock accidental globals

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end

	local main=function()
	end

	local request=function(memo)
		local ret={}
		return ret
	end

	while true do
		local timeout=0.001 -- first receive will be 1ms or less
		repeat
			local _,memo= linda:receive( timeout , task_id ) -- wait for any memos coming into this thread
			timeout=0 -- repeat receive are instant
			main() -- probably getting called every 1ms ish
			if memo then
				local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
				if not ok then ret={error=ret or true} end -- reformat errors
				if memo.id then -- result requested
					linda:send( nil , memo.id , ret )
				end
			end
		until not memo
	end

end


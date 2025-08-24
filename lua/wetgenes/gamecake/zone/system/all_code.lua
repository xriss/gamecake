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

	-- create threads and init data and subscriptions

	local oven=scene.oven

	-- db load/save interface ( boot data )
	oven.tasks:add_global_thread({
		count=1,
		id="all_db",
		code=all.code.db,
	})

	-- tweens
	oven.tasks:add_global_thread({
		count=1,
		id="all_tweens",
		code=all.code.tweens,
	})
	oven.tasks:do_memo({
		task="all_tweens",
		id=false,
		cmd="scene",
		scene=scene.wrap_name,
	})
	oven.tasks:do_memo({
		task="all_tweens",
		id=false,
		cmd="subscribe",
		subid="all_draws",
	})

	-- values
	oven.tasks:add_global_thread({
		count=1,
		id="all_values",
		code=all.code.values,
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
		subid="all_tweens",
	})

	-- popins
	oven.tasks:add_global_thread({
		count=1,
		id="all_popins",
		code=all.code.popins,
	})
	oven.tasks:do_memo({
		task="all_popins",
		id=false,
		cmd="scene",
		scene=scene.wrap_name,
	})
	oven.tasks:do_memo({
		task="all_popins",
		id=false,
		cmd="subscribe",
		subid="all_values",
	})

	-- db subscription
	oven.tasks:do_memo({
		task="all_values",
		id=false,
		cmd="subscribe",
		subid="all_db",
	})

end


-- task that deals with database access
all.code.db=function(linda,task_id,task_idx)
	local M,all -- hide for thread safety
	local oldprint=print
	print=function(...) return oldprint(task_id,...) end
	local global=require("global") -- lock accidental globals

	local all=require("wetgenes.gamecake.zone.system.all")

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end
	
	local oven=require("wetgenes.gamecake.toaster").bake({linda=linda})

	local db=all.db.open({linda=linda,time=oven.time})

	local request=function(memo)
		local ret={}

		if memo.cmd=="values" then -- incoming values from first task
			for uid,boot in pairs(memo.values.boots) do
				boot.uid=uid
				db:save_boot(boot)
			end
		elseif memo.cmd=="get_boots" then -- read some boots from db
			ret.boots , ret.error = db:get_boots(memo.boot,memo.recursive)
		end

		return ret
	end

	while true do
		local timeout=0.001 -- first receive will be 1ms or less
		repeat
			local _,memo= linda:receive( timeout , task_id ) -- wait for any memos coming into this thread
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

-- task that deals with popin/out and other world generation
all.code.popins=function(linda,task_id,task_idx)
	local M,all -- hide for thread safety
	local oldprint=print
	print=function(...) return oldprint(task_id,...) end
	local global=require("global") -- lock accidental globals

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end

	local oven=require("wetgenes.gamecake.toaster").bake({linda=linda})
	oven.upnet=oven.rebake("wetgenes.gamecake.upnet")

	local scene
	local popins

	local main=function()
		if popins then
		end
	end

	local request=function(memo)
		local ret={}

		if memo.cmd=="scene" then
			if scene then
				scene:full_clean()
				scene=nil
				popins=nil
			end
			if memo.scene then -- can delete scene by not naming one
				scene=require(memo.scene).create()
				scene.oven=oven
				scene.subscribed={}
				scene.infos.all.scene.initialize(scene)
				print("create",memo.scene)
				popins=scene:popins({caste="chunk",scene=scene})
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

		elseif memo.cmd=="first" then
			ret.pops=popins:first(memo)
		end

		return ret
	end

	while true do
		main() -- probably getting called every 1ms ish
		local timeout=0.001 -- first receive will be 1ms or less
		repeat
			local _,memo= linda:receive( timeout , task_id ) -- wait for any memos coming into this thread
			timeout=0 -- repeat receive are instant so we can empty the queue
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
	local oldprint=print
	print=function(...) return oldprint(task_id,...) end
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
			local undo_count,update_count=scene:do_update_values()
			if update_count~=0 or undo_count~=0 then
--				print("undo",-undo_count,"update",update_count,"tick",scene.values:get("tick"),scene.values:get("tick_input"))
			end
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
				else
					scene:systems_cocall("setup")
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
			if memo then
				local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
				if not ok then ret={error=ret or true} end -- reformat errors
				if memo.id then -- result requested
					linda:send( nil , memo.id , ret )
				end
			end
			main() -- probably getting called every 1ms ish
		until not memo
	end

end

-- manage scene predictions and draw state generation ( draw needs to happen on main thread )
all.code.tweens=function(linda,task_id,task_idx)
	local M,all -- hide for thread safety
	local oldprint=print
	print=function(...) return oldprint(task_id,...) end
	local global=require("global") -- lock accidental globals

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end
	
	-- basic oven, nogl etc
	local oven=require("wetgenes.gamecake.toaster").bake({linda=linda})
	oven.upnet=oven.rebake("wetgenes.gamecake.upnet")

	local scene
	local wait_for_values=false
	
	-- this should mostly not do anything as it will be called a lot
	local main=function()
		if scene then
			if not wait_for_values then
				local draw_count=scene:do_update_tweens()
				if draw_count~=0 then
--					print("draw",draw_count,"tick",scene.values:get("tick"),scene.values:get("tick_input"))
				end
			end
		end
	end

	local request=function(memo)
		local ret={}
		
		if memo.cmd=="values" then -- incoming values from first task
			if scene then
				-- undo draw predictions without full inputs
				if scene.values:get("tick") > scene.values:get("tick_input") then
					while scene.values[2] and scene.values:get("tick") > scene.values:get("tick_input") do
		--print("undo")
						scene:do_unpush()
					end
					scene:call("get_values") -- sync item and kinetics
					scene:call("set_body")
				end

	--print("values")
				scene:values_call( function(it) it.values:push() end )
				scene:load_all_values(memo.values)
				while scene.values[3] do -- and scene.values:get("tick",2) <= scene.ticks.agreed do
					scene:do_pull()
				end
				if wait_for_values then
					wait_for_values=false -- we got values so can continue
	--				print("wait_for_values",false)
				end
			end
		elseif memo.cmd=="scene" then
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
				else
					scene:systems_cocall("setup")
				end
			end			

		elseif memo.cmd=="subscribe" then

			scene.subscribed[memo.subid]={}
			
			local tweens=scene:save_all_tweens(true) -- first full dump
			scene.oven.tasks:do_memo({
				task=memo.subid,
				id=false,
				cmd="tweens",
				tweens=tweens,
			})

		elseif memo.cmd=="unsubscribe" then

			scene.subscribed[memo.subid]=nil

		elseif memo.cmd=="tick" then
			while scene.values[2] do
				scene:do_unpush()
			end
			scene.values:set("tick",memo.tick)
			scene.values:set("tick_input",memo.tick)
			wait_for_values=true
--			print("wait_for_values",true)
		end
		
		return ret
	end

	while true do
		local timeout=0.001 -- first receive will be 1ms or less
		repeat
			local _,memo= linda:receive( timeout , task_id ) -- wait for any memos coming into this thread
			timeout=0 -- repeat receive are instant
			if memo then
				local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
				if not ok then ret={error=ret or true} end -- reformat errors
				if memo.id then -- result requested
					linda:send( nil , memo.id , ret )
				end
			end
			main() -- probably getting called every 1ms ish
		until not memo
	end

end


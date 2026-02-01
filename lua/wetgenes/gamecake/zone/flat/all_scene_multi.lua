--
-- (C) 2025 Kriss@XIXs.com
--

-- multi player
-- multi task
-- multiple updates with rewinds and resyncs ( this will confuse any printed debug )
-- tween draw data with network updates run at 16fps

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.import=function(_,all)
all=all or {}
-- sub tasks for running in other threads
all.code=all.code or {}
-- methods added to manifest, we do not require a scene or systems to manifest boot data
all.manifest=all.manifest or {}
-- methods added to scene
all.scene=all.scene or {}
-- methods added to systems, shared resources can be kept in a system but not state data
all.system=all.system or {}
-- methods added to each item
all.item=all.item or {}

local Ox=function(n) return string.format("%012x",n or 0) end

local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

local bit = require("bit")

local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")

local json_diff=require("wetgenes.json_diff")
local hashish=require("wetgenes.json_diff").hashish


all.scene.do_memo=function(scene,...)	return all.do_memo(scene.oven.tasks.linda,...)	end
all.scene.memos=function(scene,...)	return all.memos(scene.oven.tasks.linda,...)	end

-- make sure we keep tasks running when in cocall loops
all.scene.also_cocall=function()
	oven.tasks:update()
end

-- initalize systems in the scene
all.scene.initialize=function(scene)

	-- get some persistant storage which should only be used inside a coroutine
	local db=require("wetgenes.gamecake.zone.system.db").open()

	-- add all.scene to scene as a metatable so we can do eg scene:call_xxxxx functions
	setmetatable(scene,{__index=scene.infos.all.scene})
	-- this guarantees call order, so if we call setup we will always call setup on kinetic system/items first
	scene.sortby={
		--
		kinetic =	-1000	-20,
		render  =	-1000	-10,
		--
		input   =	0		-60,
		camera  =	0		-50,
		chunk   =	0		-40,
		player  =	0		-30,
		tool    =	0		-20,
		build   =	0		-10,
		--
		-- everything not explicitly mentioned goes here in alphabetical order
		--
		sky     =   0		+10,
		water   = 	0		+20,
		--
		zone    =	1000	+10,
		sync    =	1000	+20,
		--
	}
	scene:sortby_update()

	scene.scrib={}
	scene.db=db
	for caste,info in pairs(scene.infos) do -- initialize each system from info
		local sys={}
		sys.caste=caste
		sys.scene=scene
		sys.info=info
		sys.db=db
		scene.infos.all.system.initialize(sys)
		scene:systems_insert(sys)
	end

--	local upnet=scene.oven.upnet
--	upnet.hooks.sync=function(client,msg) return scene:recv_msg_sync(client,msg) end


	scene.ticks={}
	scene:ticks_sync()
	scene.values:set("tick",scene.ticks.now) -- starting tick
	scene.values:set("tick_input",scene.ticks.now) -- cache of latest input tick for this update
	-- init scene.ticks from upnet.ticks

	scene.oven.upnet.subscribe("upsall")
end

all.scene.ticks_sync=function(scene)
	-- keep running updates here until we move it into a subtask
 	scene.oven.upnet.update()
	for n,v in pairs( scene.oven.upnet.get_ticks() ) do scene.ticks[n]=v end
end

all.scene.do_update_values=function(scene)



	local undo_count=0
	local update_count=0

	for m in scene.oven.tasks:memos("upsall") do
		if m.need_sync then
			scene.last_need_sync = m.need_sync
			scene:send_msg_sync(m.need_sync)
		end
	end

	scene:ticks_sync()

-- reset time if we get too far out of sync
	if scene.ticks.input > scene.values:get("tick_input")+32 then
		scene.oven.upnet.reset_tick( scene.values:get("tick_input")+32 )
		scene:ticks_sync()
	end

	if scene.ticks.input > scene.values:get("tick_input") then

		-- undo draw predictions without full inputs
		if scene.values:get("tick") > scene.values:get("tick_input") then
			while scene.values[2] and scene.values:get("tick") > scene.values:get("tick_input") do
				undo_count=undo_count+1
				scene:do_unpush()
			end
			scene:call("get_values") -- sync item and kinetics
			scene:call("set_body")
		end

		while scene.ticks.input > scene.values:get("tick") do -- update with full inputs and save hashs
		

			update_count=update_count+1
			if scene.oven.console then
				scene.oven.console.display_disable=false
				scene.oven.console.display_clear()
				display("")
			end

			scene:do_push() -- inc tick
			scene.ups=scene.oven.upnet.get_ups( scene.values:get("tick") )

			-- called often...

			scene:systems_call("update")
			scene:call("update")
			scene:call("update_kinetic")
			scene.values:set("tick_input", scene.values:get("tick") )
			
			-- save hashes 
--			local hash=scene:get_hashs( scene.values:get("tick") )[1]
local hash=0
			scene.oven.upnet.set_hash( scene.values:get("tick") , hash )

			if scene.oven.console then
				scene.oven.console.display_disable=true
			end
			
			-- disable house keeping for a while
			if scene.pause_housekeeping then
				if scene.pause_housekeeping>0 then
					scene.pause_housekeeping=scene.pause_housekeeping-1
				else
					scene:systems_call("housekeeping")
				end
			else
				scene:systems_call("housekeeping")
			end
			-- this should only be set in the values sub task
			if scene.subscribed then -- send out new values
				local values=scene:save_all_values()
				for subid in pairs(scene.subscribed) do
					scene.oven.tasks:do_memo({
						task=subid,
						id=false,
						cmd="values",
						values=values,
					})
				end
			end
			
			scene:ticks_sync()
		end

	-- this also needs to be synced with upnet using subsciptions?
--	print( "CC", #scene.values, scene.values[2] and scene.values:get("tick",2) , scene.ticks.agreed )
	-- merge slot 1+2 when that data has been sync confirmed
		while scene.values[3] do -- and scene.values:get("tick",2) <= scene.ticks.agreed do
			scene:do_pull()
		end

-- test reloading everything every frame
--		local dat=scene:save_all_values(true)
--		scene:call(function(it)if it.caste~="kinetic" then it:destroy() end end)
--		scene:load_all_values(dat)

	end
	
	return undo_count , update_count
end

all.scene.do_update_tweens=function(scene)

	local draw_count=0

	scene:ticks_sync()

	local need_tween_push=( scene.tweens:get("tick") or 0 ) < ( scene.values:get("tick") or 0 )
	while  scene.ticks.now >= scene.values:get("tick") do -- predict until we are in the future
--		if draw_count>50 then
--			print("overdraw",scene.ticks.now , scene.values:get("tick"))
--			break
--		end
		need_tween_push=true

		draw_count=draw_count+1
		scene:do_push()
		scene.ups=scene.oven.upnet.get_ups( scene.values:get("tick") )

		scene:systems_call("update")
		scene:call("update")
		scene:call("update_kinetic")

		scene:ticks_sync()
	end
	
	-- copy some extra values into the tweens
	if need_tween_push or #scene.tweens<2 then
	
		scene:values_call( function(it)
			it.tweens:push() -- fresh slot
			for n,v in pairs(it.values[1]) do -- slot 1 will have all names
				it.tweens:set( n , it.values:get(n) ) -- fill up fresh slot
			end
		end )

		-- this should only be set in the values sub task
		if scene.subscribed then -- send out new values
			local tweens=scene:save_all_tweens()
			for subid in pairs(scene.subscribed) do
				scene.oven.tasks:do_memo({
					task=subid,
					id=false,
					cmd="tweens",
					tweens=tweens,
				})
			end
		end
	end
	
	-- shrink tweens down to two slots so we may tween between them
	while #scene.tweens > 2 and scene.tweens:get("tick",2) <= scene.ticks.now do
		scene:values_call( function(it) it.tweens:pull() end )
	end
	while #scene.tweens > 2 do
		scene:values_call( function(it) it.tweens:merge() end )
	end
	
	return draw_count
end

all.scene.do_update=function(scene)


	scene:ticks_sync()
--	if scene.ticks.now < scene.values:get("tick") then return end -- tick advance only?

local counts={}
counts.undo=0
counts.update=0
counts.draw=0


	scene.tween=nil -- disable tweening while updating

--	main_zone.scene.call("update")

--	local upnet=scene.oven.upnet
--	if oven.upnet_pause=="catchup" then -- fast forward time
--		upnet.update() -- do not advance time
--	else
--	end


--[[
	for memo in oven.tasks:memos("all_test") do
		if memo.cmd=="values" then

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
			scene:load_all_values(memo.values)
		end
	end
]]

--	counts.undo , counts.update = scene:do_update_values()

	if not scene.scrib.tim then
		scene.scrib.tim=oven.times.create()
	end
	scene.scrib.tim.start()

	scene.scrib.tween=0
	scene.scrib.pull=0
	scene.scrib.merge=0
	for memo in oven.tasks:memos("all_draws") do
		if memo.cmd=="tweens" then
			scene:values_call( function(it) it.tweens:push() end )
			scene:load_all_tweens(memo.tweens)
			scene.scrib.tween=scene.scrib.tween+1
		end
	end
	scene:ticks_sync()
	-- shrink tweens down to two slots so we may tween between them
	while #scene.tweens > 2 and scene.tweens:get("tick",2) <= scene.ticks.now do
		scene:values_call( function(it) it.tweens:pull() end ) -- merge 2 into 1
		scene.scrib.pull=scene.scrib.pull+1
	end
	while #scene.tweens > 2 do
		scene:values_call( function(it) it.tweens:merge() end ) -- merge 3 into 2
		scene.scrib.merge=scene.scrib.merge+1
	end

	scene.scrib.tim.stop()

--if #scene.tweens==2 then
--print( scene.tweens:get("tick",1) , scene.tweens:get("tick",2) , scene.ticks.time )
--end

--	counts.draw = scene:do_update_tweens()


--upnet.print( upnet.ticks.input , upnet.ticks.update , upnet.ticks.now , upnet.ticks.draw )

--	print( "do_update" , -counts.undo , counts.update , "+"..counts.draw , scene.ticks.time , #scene.values , #scene.tweens )

-- this slows things down to test worst case updates
--collectgarbage()

end

all.scene.do_draw=function(scene)

	scene:ticks_sync()

	scene.scrib.tim.done()
--[[
if scene.scrib.tim.time > 0.01 then
PRINT("TIM",math.floor(scene.scrib.tim.time*1000))
end
]]

--	local upnet=scene.oven.upnet
	oven.console.lines_display[2]=(
--	"tween:"..scene.scrib.tween.." pull:"..scene.scrib.pull.." merge:"..scene.scrib.merge.." "..
	"now:"..scene.ticks.now.." inp:"..scene.ticks.input.." agr:"..scene.ticks.agreed.." bse:"..scene.ticks.base)

--	local nowtick=upnet.nowticks()
--	print("do_draw",scene.ticks.time)
	local t=scene.ticks.time
	local n=math.floor(t)
	local f=t-n

	if #scene.tweens == 2 and scene.tweens:get("tick",1) then
		scene.tween=scene.ticks.time-scene.tweens:get("tick",1)
	else
		scene.tween=1
	end
	if scene.tween<0 then scene.tween=0 end -- sanity
	if scene.tween>1 then scene.tween=1 end

	scene:call("render_camera")

	scene:call("render_screen")


--	print( "do_draw" , scene.ticks.time)

end

all.scene.recv_msg_sync=function(scene,client,msg)

--	local upnet=scene.oven.upnet
	local tick=msg.sync
	local topidx=1+tick-scene.ticks.base

--print( "recv sync" , upnet.us , tick , topidx , upnet.ticks.base  , #scene.values )
	if topidx > #scene.values then print( "OVER?" , topidx , #scene.values ) return end

	for uid,d in pairs( msg.uids ) do
		local it=scene.uids[uid]
		if it then
			it:load_values(d,topidx)
		end
	end

	local hash=msg.hashs[1]
--print( upnet.dmode("syncR") , tick , Ox(hash) )
	scene.oven.upnet.set_hash(tick,hash)

--print( "RECV SYNC" , tick..":"..topidx , Ox(hash) )

--dump({hashs,msg.hashs})

--[[
	local uids={}
	for uid,it in pairs( scene.uids ) do
		uids[uid]=it:save_diff(topidx)
	end

dump({uids,msg.uids})

error("stop")
]]

--[[
	while upnet.ticks.draw>upnet.ticks.update do -- undo draw prediction update
		upnet.ticks.draw=upnet.ticks.draw-1
		scene:do_unpush()
	end
]]
	while scene.values:get("tick") > tick do -- undo update prediction back to bad tick
--		upnet.ticks.update=upnet.ticks.update-1
--		upnet.ticks.draw=upnet.ticks.update
		scene:do_unpush()
	end

--	local hashs=scene:get_hashs(tick)
--	local hash=hashs[1]
--print( upnet.dmode("syncR"), tick , Ox(hash) )
--for n,v in pairs(hashs) do hashs[n]=Ox(v) end ; dump(hashs)


end

all.scene.send_msg_sync=function(scene,tick)

--	local upnet=scene.oven.upnet
	local topidx=1+tick-scene.ticks.base

	local hashs=scene:get_hashs(tick)
	local hash=hashs[1]
--print( upnet.dmode("syncS") , tick , Ox(hash) )
--for n,v in pairs(hashs) do hashs[n]=Ox(v) end ; dump(hashs)

	if topidx > #scene.values then print( "OVER?" , topidx , #scene.values ) return end

	local uids={}
	for uid,it in pairs( scene.uids ) do
		uids[uid]=it:save_diff(topidx)
	end
	local hashs=scene:get_hashs(tick)
	local hash=hashs[1]
	
	upnet.broadcast({
		sync=tick,
		uids=uids,
		hashs=hashs,
	})


--print( "send sync" , upnet.us , tick , topidx , upnet.ticks.base , Ox(hash) )
--print( "send sync" , tick..":"..topidx , Ox(hash) )


--error("stop")

end

	return all
end

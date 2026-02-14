--
-- (C) 2025 Kriss@XIXs.com
--

-- single player
-- single task
-- single updates with no rewinds or resyncs
-- but we still tween draw data and run updates at 16fps

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


-- make sure we keep tasks running when in cocall loops
all.scene.also_cocall=function()
	oven.tasks:update()
end

-- initalize systems in the scene
all.scene.initialize=function(scene)

	-- get some persistant storage which should only be used inside a coroutine
--	local db=require("wetgenes.gamecake.zone.system.db").open()

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

	-- we are not doing anythinmg clever so tweens and values can be shared
	scene.tweens=scene.values

	scene.ticks={}
	scene:ticks_sync()
	scene.values:set("tick",scene.ticks.now) -- starting tick
	scene.values:set("tick_input",scene.ticks.now) -- cache of latest input tick for this update
	-- init scene.ticks from upnet.ticks

--	scene.oven.upnet.subscribe("upsall")
end

all.scene.ticks_sync=function(scene)
	-- keep running updates here until we move it into a subtask
 	scene.oven.upnet.update()
	for n,v in pairs( scene.oven.upnet.get_ticks() ) do scene.ticks[n]=v end
end

all.scene.do_update=function(scene)

	scene.tween=nil -- disable tweening while updating

	scene:ticks_sync()
-- reset time if we get too far out of sync
	if scene.ticks.input > scene.values:get("tick_input")+2 then
		scene.oven.upnet.reset_tick( scene.values:get("tick_input")+2 )
		scene:ticks_sync()
	end

	if scene.ticks.input > scene.values:get("tick_input") then

		while scene.ticks.input > scene.values:get("tick") do -- update with full inputs and save hashs

			scene:do_push() -- inc tick
			scene.ups=scene.oven.upnet.get_ups( scene.values:get("tick") )

			scene:systems_call("update")
			scene:call("update")
			scene:call("update_kinetic")
			scene.values:set("tick_input", scene.values:get("tick") )

local hash=(scene.ticks.input)%0x100000000 -- fake hash
			scene.oven.upnet.set_hash( scene.values:get("tick") , hash )

			scene:ticks_sync()
		end

	-- merge slot 1+2 when that data has been sync confirmed
		while scene.values[3] do -- and scene.values:get("tick",2) <= scene.ticks.agreed do
			scene:do_pull()
		end

	end

end

all.scene.do_draw=function(scene)

	-- note tweens and values are the same
	if #scene.tweens == 2 and scene.tweens:get("tick",1) then
		scene.tween=scene.ticks.time-scene.tweens:get("tick",1)
	else
		scene.tween=1
	end
	if scene.tween<0 then scene.tween=0 end -- sanity
	if scene.tween>1 then scene.tween=1 end

--PRINT(scene.tween)

	scene:systems_call("draw")
	scene:call("draw")

end


	return all
end

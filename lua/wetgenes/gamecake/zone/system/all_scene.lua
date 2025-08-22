--
-- (C) 2025 Kriss@XIXs.com
--

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local all=require("wetgenes.gamecake.zone.system.all") -- put everything in here
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

M.scene=all.scene -- our only export is also all.scene


local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local Ox=function(n) return string.format("%012x",n or 0) end

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
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

all.scene.create=function(scene,boot) -- create from boot
	return scene.systems[ boot.caste or boot[1] ]:create(boot)
end
all.scene.gene=function(scene,boot) -- cleanup boot making sure it is well formed
	return scene.systems[ boot.caste or boot[1] ]:gene(boot)
end

all.scene.creates=function(scene,boots) -- batch create and set depends
	local items={}
	-- create
	for i,boot in ipairs(boots) do
		if boot.uid then -- pre existing check (we probably just want to fix the depends)
			items[i]=scene:find_uid(boot.uid)
			-- should we also apply the boot here?
			-- not sure, maybe just assert on diffs?
		end
		if not items[i] then -- we do need to create
			items[i]=scene:create(boot)
		end
	end
	-- assign depends to uids
	for i,boot in ipairs(boots) do
		if boot.depends then
			for name,idx in pairs(boot.depends) do 
				items[i]:depend(name,items[idx].uid) -- convert idx into uid
			end
		end
	end
	-- return all created items
	return items
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
		kinetic =	-1000	-2,
		render  =	-1000	-1,
		--
		input   =	0		-5,
		camera  =	0		-4,
		player  =	0		-3,
		tool    =	0		-2,
		build   =	0		-1,
		--
		-- everything not explicitly mentioned goes here in alphabetical order
		--
		sky     =   0		+1,
		water   = 	0		+2,
		--
		zone    =	1000	+1,
		sync    =	1000	+2,
		--
	}
	scene:sortby_update()

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

			scene:systems_call("update")
			scene:call("update")
			scene:call("update_kinetic")
			
			-- save hashes 
			scene.values:set("tick_input", scene.values:get("tick") )
--			local hash=scene:get_hashs( scene.values:get("tick") )[1]
local hash=0
			scene.oven.upnet.set_hash( scene.values:get("tick") , hash )

			if scene.oven.console then
				scene.oven.console.display_disable=true
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

-- called often but out of sync of rewinds so will probably cause a resync
-- this is chugging and needs to happen out of state or something
--	scene:systems_call("housekeeping")


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

	for memo in oven.tasks:memos("all_draws") do
		if memo.cmd=="tweens" then
			scene:values_call( function(it) it.tweens:push() end )
			scene:load_all_tweens(memo.tweens)
		end
	end
	scene:ticks_sync()
	-- shrink tweens down to two slots so we may tween between them
	while #scene.tweens > 2 and scene.tweens:get("tick",2) <= scene.ticks.now do
		scene:values_call( function(it) it.tweens:pull() end ) -- merge 2 into 1
	end
	while #scene.tweens > 2 do
		scene:values_call( function(it) it.tweens:merge() end ) -- merge 3 into 2
	end

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

--	local upnet=scene.oven.upnet
	oven.console.lines_display[2]=("now:"..scene.ticks.now.." inp:"..scene.ticks.input.." agr:"..scene.ticks.agreed.." bse:"..scene.ticks.base)

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

all.scene.get_singular=function(scene,name)
	local sys=scene.systems[name]
	if sys then return sys.singular end
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


all.scene.get_hashs=function(scene,tick)

--	local upnet=scene.oven.upnet
	local topidx=1+tick-scene.ticks.base
	if topidx > #scene.values or topidx<1 then print( "OVER2?" , tick, topidx , #scene.values ) ; return {} end

	local r={0}
	for _,sys in ipairs(scene.systems) do -- hash each sytem
		local hash=0
		local items=scene.data[ sys.caste ]
		for _,it in ipairs( items or {} ) do -- hash each item
			local top=it.values[topidx]
			if next(top) then -- need some values, mostly this will be empty
				local chksum=0 -- it.values[topidx] and it.values[topidx].chksum
				local ns={}
				for n,v in pairs(top) do ns[#ns+1]=n end
				table.sort(ns)
				for i,n in ipairs(ns) do chksum=hashish(chksum,top[n]) end
				if chksum then
					hash=json_diff.hashish(hash,chksum)
				end
			end
		end
		r[sys.caste]=hash
		r[1]=json_diff.hashish(r[1],hash)
	end

	return r
end

all.scene.do_pull=function(scene)
	scene.values:pull()
	scene:call("pull")
end

all.scene.do_push=function(scene)
	scene.values:push()
	scene.values:set( "tick" , scene.values:get("tick")+1 ) -- advance tick
	scene:call("push")
end

all.scene.do_unpush=function(scene)
	scene.values:unpush()
	-- need to unpush and also delete items...
	local uid=scene.values:get("uidtop")
	scene:call(function(it)
		if it.uid and it.uid>uid then -- item is from the future so delete
			it:destroy()
		else
			it:unpush()
		end
	end)
end




all.scene.save_all_values=function(scene,force_full_subscription)
	local ret={}
	ret.boots={}

	local zone=scene:get_singular("zone") -- current zone ( all current items should belong to this zone )
--	ret.zid=zone and zone.uid or 0
--	ret.zname=zone and zone.name or ""

	
	local save=function(it,caste,uid)
		local r
		if force_full_subscription or it.values_are_new then
			it.values_are_new=nil
			r=it.values:save_all()
			r.caste=caste -- caste is not in tweens but is required for booting a new item
			-- we can junk some of these values as they will be filled with defaults
			-- so we can mess with values
			r.notween=nil -- do not need
			r.deleted=r.deleted or nil -- turn false into a nil
--			r.zid=nil -- zone should be a constant for every item dumped
--			r.zname=nil -- also constant
--			if not ( r.uids and next(r.uids) ) then r.uids=nil end -- remove empty uids
		else
			r=it.values[#it.values]
		end
		ret.boots[uid]=r
	end

	save(scene,"scene",1)
	scene:call(function(it) save(it,it.caste,it.uid) end)
	
	return ret
end

all.scene.load_all_values=function(scene,dat)

	-- set scene values
	for n,v in pairs(dat.boots[1]) do
		scene.values:set(n,v)
	end

	-- delete items
	scene:call(function(it)
		if not dat.boots[it.uid] then -- all live objects will be in this map
			it:destroy()
		end
	end)

	-- update or create items
	local boots={}
	for uid,boot in pairs(dat.boots) do
		if uid>1000 then -- special ids bellow 1000
			local it=scene:find_uid(uid)
			if it then -- load into existing object
				for n,v in pairs(boot) do
					it.values:set(n,v)
				end
			else -- need to create new
				boots[#boots+1]=boot
				boot.uid=uid
				if type(boot.zip)=="string" then boot.zip=all.uncompress(boot.zip) end
			end
		end
	end	
	-- must create in uid order
	table.sort(boots,function(a,b) return a.uid<b.uid end)
	for i,boot in ipairs(boots) do
		scene:create(boot)
	end

end

all.scene.save_all_tweens=function(scene,force_full_subscription)
	local ret={}
	ret.boots={}
	local save=function(it,caste,uid)
		local r
		if force_full_subscription or it.values_are_new then
			it.values_are_new=nil
			r=it.tweens:save_all()
			r.caste=caste -- caste is not in tweens but is required for booting a new item
			r.notween=nil -- might be bad?
			r.deleted=r.deleted or nil -- turn false into a nil
		else
			r=it.tweens[#it.tweens]
		end
		ret.boots[uid]=r
	end

	save(scene,"scene",1)
	scene:call(function(it) save(it,it.caste,it.uid) end)
	
	return ret
end

all.scene.load_all_tweens=function(scene,dat)

	-- set scene values
	for n,v in pairs(dat.boots[1]) do
		scene.tweens:set(n,v)
	end

	-- delete items
	scene:call(function(it)
		if not dat.boots[it.uid] then -- all live objects will be in this map
			it:destroy()
		end
	end)
	
	-- update or create items
	local boots={}
	for uid,boot in pairs(dat.boots) do
		if uid>1000 then
			local it=scene:find_uid(uid)
			if it then -- load into existing object
				for n,v in pairs(boot) do
					it.tweens:set(n,v)
				end
			else -- need to create new
				boots[#boots+1]=boot
				boot.uid=uid
				if type(boot.zip)=="string" then boot.zip=all.uncompress(boot.zip) end
			end
		end
	end	
	-- must create in uid order
	table.sort(boots,function(a,b) return a.uid<b.uid end)
	for i,boot in ipairs(boots) do
		scene:create(boot)
	end

end

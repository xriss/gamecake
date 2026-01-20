--
-- (C) 2025 Kriss@XIXs.com
--

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

all.scene.create=function(scene,boot) -- create from boot
	return scene.systems[ assert(boot.caste or boot[1]) ]:create(boot)
end
all.scene.gene=function(scene,boot) -- cleanup boot making sure it is well formed
	return scene.systems[ assert(boot.caste or boot[1]) ]:gene(boot)
end

all.scene.creates=function(scene,boots) -- batch create and set depends
	local items={}
	-- create
	for i,boot in ipairs(boots) do
		if boot then -- ignore padding values that are set to false
			if boot.uid then -- pre existing check (we probably just want to fix the depends)
				items[i]=scene:find_uid(boot.uid)
				-- should we also apply the boot here?
				-- not sure, maybe just assert on diffs?
				-- i think it is best to ignore if it exists
			end
			if not items[i] then -- we do need to create
				items[i]=scene:create(boot)
			end
		end
	end
	-- assign depends to uids
	for i,boot in ipairs(boots) do
		if boot then -- ignore padding values that are set to false
			if boot.depends then
				for name,idx in pairs(boot.depends) do 
					if items[idx] then -- can link
						items[i]:depend(name,items[idx].uid) -- convert idx into uid
					end
				end
			end
		end
	end
	-- return all created items
	return items
end


all.scene.get_singular=function(scene,name)
	local sys=scene.systems[name]
	if sys then return sys.singular end
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
--		PRINT(i,boot,boot.uid,scene:find_uid(uid),boot.caste,boot[1])
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
--		PRINT(i,boot,boot.caste,boot[1])
		scene:create(boot)
	end

end

	return all
end

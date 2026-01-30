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


-- get a system by name
all.item.get_system=function(it,name)
	return it.scene.systems[name]
end

-- get a system.singular by name
all.item.get_singular=function(it,name,idx)
	local d=it.scene.data[name]
	return d and d[idx or 1]
end


-- get or set a dependency, returns the item from the named slot
-- if uid is given then we first set the named slot to that uid
all.item.depend=function(it,name,uid)
	local uids=it:get("uids")
	local idx=tonumber(name) or it.uidmap[name] -- use number or convert string
	if not idx then return nil end -- bad name or number

	if uid and uids[idx]~=uid then -- we are setting
		local nuids={} -- new table so it is flaged as a change of data
		for i=1,it.uidmap.length do -- must be set to number of slots
			nuids[i]=uids[i] or 0 -- copy/fill array so there are no holes
		end
		uids=nuids -- replace table
		uids[idx]=uid -- change value
		it:set("uids",uids) -- write
	end

	uid=uids[idx] -- get actual uid

	if uid and uid>0 then -- valid slot with a valid uid
		return it.scene:find_uid( uid ) -- return actual item
	end

	return nil -- no item
end

all.item.destroy=function(it)
	return it.sys:destroy(it)
end

all.item.setup=function(it,boot)
	boot=boot or it.boot
end

all.item.setup_values=function(it,boot)
	boot=boot or it.boot

	it.values_are_new=true -- flag to help with value copying in subscriptions

	it.zips={} -- dupe zip cache
	for n,v in pairs( it.sys.zips or {} ) do it.zips[n]=v end
	it.values=it.scene.create_values()
-- shared values with tweens if not networking
	if it.scene.tweens==it.scene.values then
		it.tweens=it.values
	end
--	it.tweens=it.scene.create_values() -- ( drawing cache of values )

	it:set_boot(boot)

	if not boot.zid then -- no explicit zone so
		local zone=it:get_singular("zone") -- auto assign to current zone
		it:set("zid",zone and zone.uid or 0)
	end

	for n,v in pairs(it.values[1]) do -- copy base values into tweens
		it.tweens:set(n,v)
	end
	for i=1,#it.scene.tweens do -- make sure we have same tweens depth as everyone else
		if not it.tweens[i] then it.tweens[i]={} end
	end
	for i=1,#it.scene.values do -- make sure we have same values depth as everyone else
		if not it.values[i] then it.values[i]={} end
	end

end

all.item.get_auto_values=function(it)
	local s=it.scene.tween -- cache for later use
	if s then -- we are drawing, need to use tween cache
		for n,t in pairs(it.sys.types) do
			if     t=="get"   then it[n]=it.tweens:get( n )
			elseif t=="tween" then it[n]=it.tweens:tween( n , s)
			elseif t=="twrap" then it[n]=it.tweens:twrap( n , it.sys.twraps[n] , s )
			end
		end
	else
		for n,t in pairs(it.sys.types) do
			if     t=="get"   then it[n]=it.values:get( n )
			elseif t=="tween" then it[n]=it.values:get( n )
			elseif t=="twrap" then it[n]=it.values:get( n )
			end
		end
	end
	it:get_zips()
end
all.item.get_values=function(it)
	it:get_auto_values()
end

-- this should never be called by draw code so has no need to check
all.item.set_auto_values=function(it)
	for n,t in pairs(it.sys.types) do
		it:set(n,it[n])
	end
	it:set_zips()
end
all.item.set_values=function(it)
	it:set_auto_values()
end

all.item.set_boot=function(it,boot)
	boot=boot or it.boot

	for k,v in pairs( it.sys.info.values ) do
		if type(v)=="table" then
			if v.new then
				it.values:set(k,v.new( boot[k] or v )) -- copy tardis values
			else
				it.values:set(k, all.compress(boot[k]) ) -- assume uncompressed zip value
			end
		else
			it.values:set(k,boot[k] or v) -- probbaly a number, bool or string
		end
	end
end

all.item.get_boot=function(it)
	local boot={}

	for k,v in pairs( it.sys.info.values ) do
		if type(v)=="table" then
			if v.new then
				boot[k]=v.new( it.values:get(k) ) -- copy tardis values
			else
				boot[k]=all.uncompress( it.values:get(k) ) -- assume zip value
			end
		else
			boot[k]=it.values:get(k) -- probbaly a number, bool or string
		end
	end

	boot.uid=it.uid
	boot.caste=it.caste

	return boot
end

all.item.pull=function(it)
	return it.values:pull()
end
--all.system.pull=all.item.pull

all.item.push=function(it)
	it.values:push()
end

all.item.unpush=function(it)
	return it.values:unpush()
end

all.item.set=function(it,name,value)
	return it.values:set(name,value)
end

all.item.get=function(it,name,topidx)
	return it.values:get(name,topidx)
end


-- all of the tween get/set code here will check scene.tween
-- they will only work on the tweens if scene.tween is set
-- if you always require absolute access then you must call
-- it.tweens:get(...) and it.tweens:set(...) etc explicitly

-- tween version of value set
-- but only when scene.tween is set otherwise this is just a set
all.item.tset=function(it,name,value)
	if it.scene.tween then
		return it.tweens:set(name,value)
	else
		return it.values:set(name,value)
	end
end

-- tween version of value get
-- but only when scene.tween is set otherwise this is just a get
all.item.tget=function(it,name,topidx)
	if it.scene.tween then
		return it.tweens:get(name,topidx)
	else
		return it.values:get(name,topidx)
	end
end

-- tween values now(1) with previous(0) frame
-- but only when scene.tween is set otherwise this is just a get
all.item.tween=function(it,name,tween)
	if it.scene.tween then
		return it.tweens:tween(name,tween or it.scene.tween)
	else
		return it.values:get(name)
	end
end

-- tween wrap values now(1) with previous(0) frame numbers will wrap  0>= n <nmax
-- but only when scene.tween is set otherwise this is just a get
all.item.twrap=function(it,name,nmax,tween)
	if it.scene.tween then
		return it.tweens:twrap(name,nmax,tween or it.scene.tween)
	else
		return it.values:get(name)
	end
end

all.item.save_all=function(it,topidx)
	return it.values:save_all(topidx)
end

all.item.save_diff=function(it,topidx)
	return it.values:save_diff(topidx)
end

-- we want to keep these values saneish so as to avoid too much drift between clients physics
local floor=math.floor
local snap=function(n) return floor((n*0x1000)+0.5)/0x1000 end
--local snaq=function(n) return floor((n*0x01000000)+0.5)/0x01000000 end

--local snap=function(n) return n end -- return floor((n*0x1000)+0.5)/0x1000 end

all.item.set_body=function(it)
	if not it.body then return end -- no body

	it.body:position(it.pos[1],it.pos[2])
	it.body:velocity(it.vel[1],it.vel[2])
	
	if it.acc then
		it.body:force(it.acc[1],it.acc[2])
	end

--[[
	it.body:transform(
		(it.pos[1]) , (it.pos[2]) , (it.pos[3]) ,
		(it.rot[1]) , (it.rot[2]) , (it.rot[3]) , (it.rot[4]) )
	it.body:velocity( (it.vel[1]) , (it.vel[2]) , (it.vel[3]) )
	it.body:angular_velocity( (it.ang[1]) , (it.ang[2]) , (it.ang[3]) )

	if it.body_active then it.body:active(true) end -- keep this body active
]]

end

all.item.get_body=function(it)
	if not it.body then return end -- no body

	local px,py=it.body:position()
	local vx,vy=it.body:velocity()

	it.pos=V3(px,py,0)
	it.vel=V3(vx,vy,0)

	if it.acc then
		local ax,ay=it.body:force()
		it.acc=V3(ax,ay,0)
	end

--[[
	local px,py,pz,qx,qy,qz,qw=it.body:transform()
	it.pos=V3(snap(px),snap(py),snap(pz))
	it.rot=Q4(snap(qx),snap(qy),snap(qz),snap(qw))
	it.rot:normalize()

	local vx,vy,vz=it.body:velocity()
	it.vel=V3( snap(vx),snap(vy),snap(vz) )

	local ax,ay,az=it.body:angular_velocity()
	it.ang=V3( snap(ax),snap(ay),snap(az) )
]]

end

-- ( set , impulse , get ) on body this should only update the velocitys
--[[
all.item.impulse_body=function(it,v,p)
	if not it.body then return end -- no body
	if not it.vel then return end -- no body values
	if not it.ang then return end -- no body values

	it.vel=it:get("vel")
	it.ang=it:get("ang")

	it.body:velocity( (it.vel[1]) , (it.vel[2]) , (it.vel[3]) )
	it.body:angular_velocity( (it.ang[1]) , (it.ang[2]) , (it.ang[3]) )

	if p then
		it.body:impulse( v[1],v[2],v[3] , p[1],p[2],p[3] )
	else
		it.body:impulse( v[1],v[2],v[3] )
	end
	it.body:active(true)

	local vx,vy,vz=it.body:velocity()
	it.vel=V3( snap(vx),snap(vy),snap(vz) )

	local ax,ay,az=it.body:angular_velocity()
	it.ang=V3( snap(ax),snap(ay),snap(az) )

	it:set("vel",it.vel)
	it:set("ang",it.ang)

end
]]


all.item.set_zips=function(it)
	if not it.zips then return end -- no zipsge

	for n,s in pairs(it.zips) do
		local t=it[n]
		if t.dirty then -- flaged as changed by user code when we change anything
			t.dirty=false
			local z=all.compress(t)
			it.zips[n]=z
			it:set(n,z) -- set will check against current value so this is always safe to call
		end
	end

end

all.item.get_zips=function(it)
	if not it.zips then return end -- no zips

	for n,s in pairs(it.zips) do
		local z=it:get(n)
		if s~=z then -- changed
			it.zips[n]=z -- remember
			local t=all.uncompress(z)
			it[n]=t
		end
	end
end


all.item.load_values=function(it,data,topidx)

	local b=it.values[1]
	local t={}
	it.values[topidx]=t
--	if not t then print("load top",topidx,#it.values) end
	for k,v in pairs(data) do
		if type(b[k])=="table" then -- could be a special calss
			if b[k].new then -- it is a class
				t[k]=b[k].new(v) -- so enable all the meta
			else
				t[k]=v -- just a table, maybe an uncompressed zip?
			end
		else
			t[k]=v -- a non table value
		end
--print(uid,k,t[k])
	end

end
all.system.load_values=all.item.load_values

	return all
end

--
-- (C) 2024 Kriss@XIXs.com
--
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

local cmsgpack=require("cmsgpack")
local zlib=require("zlib")
local zipinflate=function(d) return d and ((zlib.inflate())(d))          end
local zipdeflate=function(d) return d and ((zlib.deflate())(d,"finish")) end
local   compress=function(d) return d and zipdeflate(cmsgpack.pack(d))   end
local uncompress=function(d) return d and cmsgpack.unpack(zipinflate(d)) end

-- iterate over caste subnames so we can merge inheritences, set reverse flag to iterate backwards
local inherits=function(names,reverse,split)
	split=split or "_" -- split on underscore by default
	if reverse then -- backwards
		return function(names,name)
			if not name then return names end -- first
			local eman=string.reverse(name)
			local u=string.find(eman,split,1,true) -- next _ from end
			if not u then return nil end -- finished
			return string.sub(name,1,-u-1)
		end,names,nil
	else -- forwards
		return function(names,name)
			if name==names then return nil end -- finshed
			local u=string.find(names,split,#name+2,true) -- next _ from start
			if not u then return names end -- final full string
			return string.sub(names,1,u-1)
		end,names,""
	end
end
--for s in inherits("a_b_c_d_e_f",true) do print(s) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local all=M

all.caste="all"

-- note that first slot is always parent and should only be used if that makes sense
all.uidmap={
	parent=1,	-- we are a child of this object
	length=0,	-- size of this array
}

all.values={
	chksum=0,		-- chksum of data changed as data is modified
	notween=true,	-- disable tweening for when an object needs to jump to a new position
	deleted=false,	-- delayed deletion of an object, when this is set object should be considered dead
	zid=0,			-- a uid of the zone this item belongs in, 0 is a synonym for null
	zname="",		-- a unique name within the zid name space
	uids=V0(),		-- list of uids related to this object [1] is always parent
}

all.types={
	chksum="ignore",
	notween="ignore",
	deleted="ignore",
	zid="ignore",
	zname="ignore",
	uids="ignore",
	-- body values use get_body_values if you need them
	pos="ignore",
	rot="ignore",
	vel="ignore",
	ang="ignore",
}


-- methods added to manifest, we do not require a scene or systems to manifest boot data
all.manifest={}

-- methods added to scene
all.scene={}

-- methods added to systems, shared resources can be kept in a system but not state data
all.system={}

-- methods added to each item
all.item={}

-- create and run or resume a cached coroutine
-- this create a coroutine called name.."_coroutine" in base
-- this represents state that can not be rewound so should only be used during housekeeping or setup
all.run_as_coroutine=function(base,name,complete)
	local name_coroutine=name.."_coroutine"
	-- finished
	if base[name_coroutine] and ( coroutine.status( base[name_coroutine] ) == "dead" ) then
		base[name_coroutine]=nil
	end
	-- startup
	if not base[name_coroutine] then
		base[name_coroutine]=coroutine.create( base[name] )
	end
	-- continue
	if base[name_coroutine] then
		repeat
			local ok,err=coroutine.resume( base[name_coroutine] , base )
			if not ok then
				print( debug.traceback( base[name_coroutine],err ) )
				os.exit(20)
			end
		until ( not complete ) or ( coroutine.status( base[name_coroutine] ) == "dead" )
	end
end
all.scene.run_as_coroutine=all.run_as_coroutine
all.system.run_as_coroutine=all.run_as_coroutine
all.item.run_as_coroutine=all.run_as_coroutine



all.manifest.create=function(manifest)
	assert(manifest)
	assert(manifest.caste)
	assert(manifest.db)
	assert(manifest.scene)

	local merge=function(into,from)
		if not into then return end
		for n,v in pairs( from or {} ) do
			if type(into[n])=="nil" then -- never overwrite
				into[n]=v
			end
		end
	end

	-- get list of all sub castes seperated by _
	local castes={}
	for caste in inherits(manifest.caste,true) do castes[#castes+1]=caste end
	castes[#castes+1]="all" -- and include all as a generic base class

	-- merge all castes
	for _,caste in ipairs(castes) do
		local info=manifest.scene:require(caste)
		if info then
			merge( manifest        , info.manifest or {} ) -- merge manifest functions
		end
	end

	return manifest
end
all.scene.manifest  =function(scene  ,manifest) return all.manifest.create(manifest) end
all.system.manifest =function(system ,manifest) return all.manifest.create(manifest) end

-- make sure we keep tasks running when in cocall loops
all.scene.also_cocall=function()
	oven.tasks:update()
end


all.scene.create=function(scene,boot) -- shorthand to create from boot
	return scene.systems[ boot.caste or boot[1] ]:create(boot)
end

-- initalize systems in the scene
all.scene.initialize=function(scene)

	-- get some persistant storage which should only be used inside a coroutine
	local db=require("wetgenes.gamecake.zone.system.db").open()

	-- add all.scene to scene as a metatable so we can do eg scene:call_xxxxx functions
	setmetatable(scene,{__index=scene.infos.all.scene})
	scene.sortby={
		--
		kinetic =	-1000	-1,
		--
		input   =	0		-5,
		camera  =	0		-4,
		player  =	0		-3,
		tool    =	0		-2,
		build   =	0		-1,
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

	local upnet=scene.oven.upnet
	upnet.hooks.sync=function(client,msg) return scene:recv_msg_sync(client,msg) end

end

all.scene.do_update=function(scene)

	if oven.upnet_pause=="updates" then return end -- only ever need one update to keep in sync

local counts={}
counts.undo=0
counts.update=0
counts.draw=0


	local upnet=scene.oven.upnet
	scene.ticklength=upnet.ticks.length
	scene.tween=1 -- disable tweening while updating


--	main_zone.scene.call("update")

	if oven.upnet_pause=="catchup" then -- fast forward time
		upnet.update() -- do not advance time
	else
		upnet.update(true) -- advance time
	end

	-- remove data we have all agreed too
	while upnet.get_client_agreed() > (upnet.ticks.base+1) do
		upnet.inc_base()
		scene:do_pull()
	end

	if upnet.need_sync then
		if upnet.mode=="host" then -- we are the host
			if upnet.need_sync ~= scene.last_need_sync then -- only send once per frame
				scene.last_need_sync = upnet.need_sync
				scene:send_msg_sync(upnet.need_sync)
			end
		end
	end

	if upnet.ticks.input>upnet.ticks.update then

		if upnet.ticks.draw>upnet.ticks.update then
			while upnet.ticks.draw>upnet.ticks.update do -- undo draw prediction update
				counts.undo=counts.undo+1
				upnet.ticks.draw=upnet.ticks.draw-1
--print("revert",upnet.ticks.update)
				scene:do_unpush()
			end
			scene:call("get_values") -- sync item and kinetics
			scene:call("set_body")
		end

		-- called often but out of sync of rewinds so will probably cause a resync
		scene:systems_call("housekeeping")

		while upnet.ticks.input>upnet.ticks.update do -- update with valid inputs
			counts.update=counts.update+1
			scene.oven.console.display_disable=false
			scene.oven.console.display_clear()
			display("")

			upnet.ticks.update=upnet.ticks.update+1
			upnet.ticks.draw=upnet.ticks.update
--print("update",upnet.ticks.update)
			scene:do_push()
--print("update",upnet.ticks.update)
			scene.ups=upnet.get_ups(upnet.ticks.update)

--print("UPDATE:"..upnet.ticks.update)

			scene:systems_call("update")
			scene:call("update")
			scene:call("update_kinetic")
			local hash=scene:get_hashs(upnet.ticks.update)[1]
--hash=0
			upnet.set_hash(upnet.ticks.update,hash)
			scene.oven.console.display_disable=true
		end

	end

--[[
	local hashs=scene:get_hashs(upnet.ticks.update)
	for n,v in pairs(hashs) do hashs[n]=Ox(v) end --; dump(hashs)
	print(upnet.dmode("H"),upnet.ticks.update,hashs[1],hashs.camera)
]]

-- predict into the future with local inputs
	local nowtick=upnet.nowticks()
	while upnet.ticks.draw<nowtick do -- update untill we are in the future
		counts.draw=counts.draw+1
		upnet.ticks.draw=upnet.ticks.draw+1
--print("future",upnet.ticks.draw)
		scene:do_push()
--print("draw",upnet.ticks.draw)
		scene.ups=upnet.get_ups(upnet.ticks.draw)

--print("UPDRAW:"..upnet.ticks.draw)

		scene:systems_call("update")
		scene:call("update")
		scene:call("update_kinetic")
	end

--upnet.print( upnet.ticks.input , upnet.ticks.update , upnet.ticks.now , upnet.ticks.draw )

--	print( "do_update" , counts.undo , counts.update , counts.draw )

-- this slows things down to test worst case updates
--collectgarbage()

end

all.scene.do_draw=function(scene)

	local upnet=scene.oven.upnet
	oven.console.lines_display[2]=("now:"..upnet.ticks.now.." inp:"..upnet.ticks.input.." agr:"..upnet.ticks.agreed.." bse:"..upnet.ticks.base)

	local nowtick=upnet.nowticks()
	scene.tween=1+nowtick-upnet.ticks.draw -- tween draw blend between frames
	if scene.tween<0 then scene.tween=0 end -- sanity
	if scene.tween>1 then scene.tween=1 end

	scene:call("render_camera")

	scene:call("render_screen")


--	print( "do_draw" )

end


-- get a system by name
all.system.get_system=function(sys,name)
	return sys.scene.systems[name]
end

-- get a system.singular by name
all.system.get_singular=function(sys,name)
	local sys=sys.scene.systems[name]
	if sys then return sys.singular end
end

all.system.get_rnd_raw=function(sys)
	local r=sys:get("rnd")
	if not r then r=0 for i=1,#sys.caste do r=r+sys.caste:byte(i) end r=r%65536 end
	r=(((r+1)*75)%65537)-1
	sys:set("rnd",r) -- note rnd is in the range 0 to 65535 inclusive
	return r
end
-- get a basic pseudo random number between 0 and 1 inclusive ( zxspectrum sequence )
-- a changes request to an integer between 1 and a inclusive (a can not be more than 65536)
-- b changes request to be a number between a and b inclusive (floating point)
all.system.get_rnd=function(sys,a,b)
	local r=sys:get_rnd_raw()
	if b then
		return a+((b-a)*(r/65535))
	elseif a then
		return 1+(r%a)
	end
	return (r/65535) -- so this may produce a 0 or 1
end

-- load glsl with same name as module if it exists
all.system.load_glsl=function(sys)

	local wzips=require("wetgenes.zips")
	local gl=sys.gl

	local nameslash=sys.info.modname:gsub("%.","/")
	local filename="lua/"..nameslash..".glsl"
	local src=wzips.readfile(filename)
	if src and gl then
		gl.shader_sources( src , filename )
--		print("GLSL LOAD",sys.info.modname,filename)
	end

end

-- initalize metatables links and load extra data ( call once as system is added )
all.system.initialize=function(sys)

	local merge=function(into,from)
		if not into then return end
		for n,v in pairs( from or {} ) do
			if type(into[n])=="nil" then -- never overwrite
				into[n]=v
			end
		end
	end

	-- make sure we have unique sub tables
	sys.methods=sys.methods or {}
	sys.metatable=sys.metatable or {}
	sys.zips=sys.zips or {}

	sys.info.types=sys.info.types or {}

	-- and methods will be available to items via metatable
	sys.metatable.__index=sys.methods

	-- keep shortcuts to oven bound data in each system
	sys.oven=sys.scene.oven
	sys.upnet=sys.oven.upnet
	sys.gl=sys.oven.gl

	-- keep shortcuts to oven bound data in each items metatable
	sys.methods.sys=sys
	sys.methods.scene=sys.scene
	sys.methods.oven=sys.oven
	sys.methods.upnet=sys.upnet
	sys.methods.gl=sys.gl

	sys.uidmap=sys.info.uidmap
	sys.methods.uidmap=sys.info.uidmap

	sys.methods.caste=sys.caste

	-- get list of all sub castes seperated by _
	local castes={}
	for caste in inherits(sys.caste,true) do castes[#castes+1]=caste end
	castes[#castes+1]="all" -- and include all as a generic base class

	-- merge all castes
	for _,caste in ipairs(castes) do
		local info=sys.scene.infos[caste]
		if info then
			merge( sys             , info.system or {} ) -- merge system functions
			merge( sys.methods     , info.item   or {} ) -- and item functions
			merge( sys.info.values , info.values or {} ) -- and default values
			merge( sys.info.types  , info.types  or {} ) -- and types of values
		end
	end

	for n,v in pairs( sys.info.values ) do -- find zip values which wil be non tardis tables and auto flag tweens
		local t=type(v)
		if t=="table" then -- a table
			if not v.new then -- not a tardis value, assume ziped data
				sys.zips[n]=""
				sys.info.types[n]="ignore" -- do not get/set these
			end
		end
		if not sys.info.types[n] then	-- any other values are set to auto
			sys.info.types[n]="get"
		end
	end
	sys.types={}
	for n,v in pairs( sys.info.types ) do
		if v~="ignore" then -- remove ignores
			sys.types[n]=v
		end
	end

	sys.info.values_order={}
	for n in pairs(sys.info.values) do sys.info.values_order[#sys.info.values_order+1]=n end
	table.sort(sys.info.values_order)

	-- global system values
	sys.values=sys.scene.create_values()
	sys.tweens=sys.scene.create_values()
	sys:get_rnd() -- seed with caste name

	-- load glsl code if it exists
	sys:load_glsl()

end


-- generate any missing boot data
all.gene_body=function(boot)

	boot=boot or {}

	boot.siz=boot.siz or {1,1,1}

	boot.pos=boot.pos or {0,0,0}
	boot.vel=boot.vel or {0,0,0}

	boot.rot=boot.rot or {0,0,0,1}
	boot.ang=boot.ang or {0,0,0} -- the axis of rotation and its magnitude

	return boot
end
all.system.gene_body=function(_,boot) return all.gene_body(boot) end

all.gene=function(boot)
	boot=boot or {}
	return boot
end
all.system.gene=function(_,boot) return all.gene(boot) end

-- core creation actions
all.system.create_core=function(sys,boot)
	local it={}
	local boot=boot or {}
	boot.caste=sys.caste -- make sure caste is correct
	it.boot=sys:gene(boot) -- apply basic boot values
	if it.boot.uid then it.uid = it.boot.uid end
	if it.boot.id  then it.id  = it.boot.id  end
	setmetatable(it,sys.metatable)
	sys.scene:add(it)
	it:setup_values()
	it:setup() -- system specific setup
	return it
end

all.system.create=function(sys,boot)
	return all.system.create_core(sys,boot)
end


-- get a system by name
all.item.get_system=function(it,name)
	return it.scene.systems[name]
end

-- get a system.singular by name
all.item.get_singular=function(it,name)
	local sys=it.scene.systems[name]
	if sys then return sys.singular end
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

	if it.clean then -- optional cleanup
		it:clean()
	end

	it.scene:remove( it ) -- it.scene should still be valid

end

all.item.setup=function(it,boot)
	boot=boot or it.boot

end

all.item.setup_values=function(it,boot)
	boot=boot or it.boot

	it.zips={} -- dupe zip cache
	for n,v in pairs( it.sys.zips or {} ) do it.zips[n]=v end
	it.values=it.scene.create_values()
	it.tweens=it.scene.create_values()

	it:set_boot(boot)

	if not boot.zid then -- no explicit zone so
		local zone=it:get_singular("zone") -- auto assign to current zone
		it:set("zid",zone and zone.uid or 0)
	end

	for i=1,#it.scene.values do -- make sure we have same depth as everyone else
		if not it.values[i] then it.values[i]={} end
	end

end

all.item.get_auto_values=function(it)
	for n,t in pairs(it.sys.types) do
		if     t=="tween" then it[n]=it:get(n)
		elseif t=="twrap" then it[n]=it:get(n)
		elseif t=="get"   then it[n]=it:get(n)
		end
	end
	it:get_zips()
end
all.item.get_values=function(it)
	it:get_auto_values()
end

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
				it.values:set(k, compress(boot[k]) ) -- assume uncompressed zip value
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
				boot[k]=uncompress( it.values:get(k) ) -- assume zip value
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
all.system.pull=all.item.pull

all.item.push=function(it)
	it.values:push()
end
all.system.push=all.item.push

all.item.unpush=function(it)
	return it.values:unpush()
end
all.system.unpush=all.item.unpush

all.item.set=function(it,name,value)

	-- auto update chksum value as we set any value even if it is not different
	local hash=it.values:get("chksum") or 0
	it.values:set("chksum",hashish(hash,value))

	return it.values:set(name,value)
end
all.system.set=all.item.set

all.item.get=function(it,name,topidx)
	return it.values:get(name,topidx)
end
all.system.get=all.item.get

-- tween version of value set
all.item.tset=function(it,name,value)
	return it.tweens:set(name,value)
end
all.system.tset=all.item.tset

-- tween version of value get
all.item.tget=function(it,name,topidx)
	return it.tweens:get(name,topidx)
end
all.system.tget=all.item.tget

-- tween values now(1) with previous(0) frame
all.item.tween=function(it,name,tween)
	return it.tweens:tween(name,tween or it.scene.tween)
end
all.system.tween=all.item.tween

-- tween wrap values now(1) with previous(0) frame numbers will wrap  0>= n <nmax
all.item.twrap=function(it,name,nmax,tween)
	return it.tweens:twrap(name,nmax,tween or it.scene.tween)
end
all.system.twrap=all.item.twrap

all.item.save_all=function(it,topidx)
	return it.values:save_all(topidx)
end
all.system.save_all=all.item.save_all

all.item.save_diff=function(it,topidx)
	return it.values:save_diff(topidx)
end
all.system.save_diff=all.item.save_diff

-- we want to keep these values saneish so as to avoid too much drift between clients physics
local floor=math.floor
local snap=function(n) return floor((n*0x1000)+0.5)/0x1000 end
--local snaq=function(n) return floor((n*0x01000000)+0.5)/0x01000000 end

--local snap=function(n) return n end -- return floor((n*0x1000)+0.5)/0x1000 end

all.item.set_body=function(it)
	if not it.body then return end -- no body

	it.body:transform(
		(it.pos[1]) , (it.pos[2]) , (it.pos[3]) ,
		(it.rot[1]) , (it.rot[2]) , (it.rot[3]) , (it.rot[4]) )
	it.body:velocity( (it.vel[1]) , (it.vel[2]) , (it.vel[3]) )
	it.body:angular_velocity( (it.ang[1]) , (it.ang[2]) , (it.ang[3]) )

	if it.body_active then it.body:active(true) end -- keep this body active

end

all.item.get_body=function(it)
	if not it.body then return end -- no body

	local px,py,pz,qx,qy,qz,qw=it.body:transform()
	it.pos=V3(snap(px),snap(py),snap(pz))
	it.rot=Q4(snap(qx),snap(qy),snap(qz),snap(qw))
	it.rot:normalize()

	local vx,vy,vz=it.body:velocity()
	it.vel=V3( snap(vx),snap(vy),snap(vz) )

	local ax,ay,az=it.body:angular_velocity()
	it.ang=V3( snap(ax),snap(ay),snap(az) )
end

-- ( set , impulse , get ) on body this should only update the velocitys
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

all.item.set_body_values=function(it)

	it:set("pos",it.pos)
	it:set("rot",it.rot)
	it:set("vel",it.vel)
	it:set("ang",it.ang)

end

all.item.get_body_values=function(it)

	it.pos=it:get("pos")
	it.rot=it:get("rot")
	it.vel=it:get("vel")
	it.ang=it:get("ang")

end

all.item.set_zips=function(it)
	if not it.zips then return end -- no zips

	for n,s in pairs(it.zips) do
		local t=it[n]
		if t.dirty then -- flaged as changed by user code when we change anything
			t.dirty=false
			local z=compress(t)
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
			local t=uncompress(z)
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

all.scene.recv_msg_sync=function(scene,client,msg)

	local upnet=scene.oven.upnet
	local tick=msg.sync
	local topidx=1+tick-upnet.ticks.base

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
	upnet.set_hash(tick,hash)

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

	while upnet.ticks.draw>upnet.ticks.update do -- undo draw prediction update
		upnet.ticks.draw=upnet.ticks.draw-1
		scene:do_unpush()
	end
	while upnet.ticks.update>tick do -- undo update prediction back to bad tick
		upnet.ticks.update=upnet.ticks.update-1
		upnet.ticks.draw=upnet.ticks.update
		scene:do_unpush()
	end

--	local hashs=scene:get_hashs(tick)
--	local hash=hashs[1]
--print( upnet.dmode("syncR"), tick , Ox(hash) )
--for n,v in pairs(hashs) do hashs[n]=Ox(v) end ; dump(hashs)


end

all.scene.send_msg_sync=function(scene,tick)

	local upnet=scene.oven.upnet
	local topidx=1+tick-upnet.ticks.base

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
	for _,client in pairs(upnet.clients) do
		if not client.us then
			local msg={}
			msg.sync=tick
			msg.uids=uids
			msg.hashs=hashs
			client:send(msg)
		end
	end


--print( "send sync" , upnet.us , tick , topidx , upnet.ticks.base , Ox(hash) )
--print( "send sync" , tick..":"..topidx , Ox(hash) )


--error("stop")

end


all.scene.get_hashs=function(scene,tick)

	local upnet=scene.oven.upnet
	local topidx=1+tick-upnet.ticks.base
	if topidx > #scene.values or topidx<1 then print( "OVER2?" , tick, topidx , #scene.values ) ; return {} end

	local r={0}
	for _,sys in ipairs(scene.systems) do -- hash each sytem
		local hash=sys.values[topidx] and sys.values[topidx].chksum or 0
		local items=scene.data[ sys.caste ]
		for _,it in ipairs( items or {} ) do -- hash each item
			local chksum=it.values[topidx] and it.values[topidx].chksum
			if chksum then
				hash=json_diff.hashish(hash,chksum)
			end
		end
		r[sys.caste]=hash
		r[1]=json_diff.hashish(r[1],hash)
	end

	return r
end

all.scene.do_pull=function(scene)
	scene.values:pull()
	scene:systems_call("pull")
	scene:call("pull")
end

all.scene.do_push=function(scene)
	scene.values:push()
	scene:systems_call("push")
	scene:call("push")
end

all.scene.do_unpush=function(scene)
	scene.values:unpush()
	scene:systems_call("unpush")
	-- need to unpush and also delete items...
	local uid=scene.values:get("uid")
	scene:call(function(it)
		if it.uid and it.uid>uid then -- item is from the future so delete
			it:destroy()
		else
			it:unpush()
		end
	end)
end


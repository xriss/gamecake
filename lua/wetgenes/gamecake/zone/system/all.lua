--
-- (C) 2025 Kriss@XIXs.com
--

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local all=M

-- database code
all.db=all.db or {}
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

-- convert data into or from a binary string
local mime=require("mime")
local cmsgpack=require("cmsgpack")
local zlib=require("zlib")
local zipinflate=function(d) return d and ((zlib.inflate())(d))          end
local zipdeflate=function(d) return d and ((zlib.deflate())(d,"finish")) end

-- use these functions on json style data only
all.compress=    function(d) return d and zipdeflate(cmsgpack.pack(d))   end
all.uncompress=  function(d) return d and cmsgpack.unpack(zipinflate(d)) end

-- these functions will catch nil inputs and return nils
all.encode=function(t)
	if not t then return nil end -- nil input outputs a nil rather than a string
	return all.compress(t)
end
all.decode=function(s)
	if not s then return nil end -- nil input outputs a nil rather than a table
	return all.uncompress(s)
end

-- sometimes we need to build text safe strings from binary data
all.b64_encode=function(it)
	local p=(#it)%3
	local pad
	if p~=0 then pad=string.rep("\0",3-p) end -- pad up
	return (mime.b64(it,pad))
end
all.b64_decode=function(it)
	local p=(#it)%4
	local pad
	if p~=0 then pad=string.rep("\0",4-p) end -- pad up
	return (mime.unb64(it,pad))
end

-- grab basic memo functions from tasks
-- these require a linda as first arg and may block
all.do_memo=require("wetgenes.tasks").do_memo
all.memos=require("wetgenes.tasks").memos

all.caste="all"

-- note that first slot is always parent and should only be used if that makes sense
all.uidmap={
	parent=1,	-- we are a child of this object
	length=0,	-- size of this array
}

all.values={
	notween=true,	-- disable tweening for when an object needs to jump to a new position
	deleted=false,	-- delayed deletion of an object, when this is set object should be considered dead
	zid=0,			-- a uid of the zone this item belongs in, 0 is a synonym for null
--	zname="",		-- a unique name within the zid name space
	uids=V0(),		-- list of uids related to this object [1] is always parent
}

all.types={
	notween="ignore",
	deleted="ignore",
	zid="ignore",
--	zname="ignore",
	uids="ignore",
	-- body values are ignored by default and need to be added by each system that uses them
	pos="ignore",
	rot="ignore",
	vel="ignore",
	ang="ignore",
}

-- iterate over caste subnames so we can merge inheritences, set reverse flag to iterate backwards
all.inherits=function(names,reverse,split)
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
	for caste in all.inherits(manifest.caste,true) do castes[#castes+1]=caste end
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
	if type(boot.zip)=="string" then -- auto uncompress string
		boot.zip=all.decode(boot.zip)
	end
	return boot
end
all.system.gene=function(_,boot) return all.gene(boot) end


-- auto merge other parts into all
require("wetgenes.gamecake.zone.system.all_scene")
require("wetgenes.gamecake.zone.system.all_system")
require("wetgenes.gamecake.zone.system.all_item")
require("wetgenes.gamecake.zone.system.all_code")
require("wetgenes.gamecake.zone.system.all_db")

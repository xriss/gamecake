-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log
local json=require("wetgenes.json")


module("wetgenes.www.any.datadef")

local wdata=require("wetgenes.www.any.data")
local cache=require("wetgenes.www.any.cache")

--------------------------------------------------------------------------------
--
-- Create a new local entity filled with initial data
-- the id can be and often is nil
--
--------------------------------------------------------------------------------
function def_create(env,srv,id)

	local ent={}
	
	ent.key={kind=env.kind(srv),id=id,notsaved=true} -- we will not know the key id until after we save
	ent.props={}
	
	local p=ent.props
	
	p.created=(srv and srv.time) or os.time()
	p.updated=(srv and srv.time) or p.created
	
	for i,v in pairs(env.default_props or {}) do
		p[i]=v
	end

	wdata.build_cache(ent) -- this just copies the props across
	
-- these are json only vars
	local c=ent.cache
	
	for i,v in pairs(env.default_cache or {}) do
		c[i]=v
	end

	return env.check(srv,ent)
end

--------------------------------------------------------------------------------
--
-- Save to database
-- this calls check before putting and does not put if check says it is invalid
-- build_props is called so code should always be updating the cache values
--
--------------------------------------------------------------------------------
function def_put(env,srv,ent,tt)
	t=tt or wdata -- use transaction?

	if not env.check(srv,ent) then return nil end -- check that this is valid to put

	wdata.build_props(ent)
	local ks=t.put(ent)
	
	if ks then
		ent.key=wdata.keyinfo( ks ) -- update key with new id
		wdata.build_cache(ent)
		
		if not tt then env.cache_fix(srv,env.cache_what(srv,ent)) end -- destroy any cache if not in transaction
	end

	return ks -- return the keystring which is an absolute name
end


--------------------------------------------------------------------------------
--
-- Load from database, pass in id or entity
-- the props will be copied into the cache
--
--------------------------------------------------------------------------------
function def_get(env,srv,id,tt)

	local ent=id
	
	if type(ent)~="table" then -- get by id
		ent=env.create(srv)
		ent.key.id=id
	end
	
	local ck=env.cache_key(srv,ent.key.id)
	if not tt and ck then -- can try for cached value outside of transactions
		local ent=cache.get(srv,ck)
		if ent then return env.check(srv,json.decode(ent)) end -- Yay, we got a cached value
	end
	
	local t=tt or wdata -- use transaction?
	
	if not t.get(ent) then
		if not tt and ck then -- kill auto cache
			cache.del(srv,ck)
		end
		return nil
	end
	
	wdata.build_cache(ent)
	
	if not tt and ck then -- auto cache ent for one hour
		cache.put(srv,ck,json.encode(ent),60*60)
	end
	
	return env.check(srv,ent)
end



--------------------------------------------------------------------------------
--
-- get - update - put
--
-- f must be a function that changes the entity and returns true on success
-- id can be an id or an entity from which we will get the id
--
--------------------------------------------------------------------------------
function def_update(env,srv,id,f)

	if type(id)=="table" then id=id.key.id end -- can turn an entity into an id
		
	for retry=1,10 do
		local mc={}
		local t=wdata.begin()
		local e=env.get(srv,id,t) -- must exist
		if e then
			env.cache_what(srv,e,mc) -- the original values
			if not e.key.notsaved then -- not a newly created entity
				if e.cache.updated>srv.time then -- stop any updates that time travel backwards
					t.rollback()
					log("DATA UPDATE FAILED TIMETRAVEL:"..env.kind(srv)..":"..id)
					return false
				end
			end
			e.cache.updated=srv.time -- the function can change this change if it wishes
			if not f(srv,e) then -- hard fail
				t.rollback()
				log("DATA UPDATE FAILED FUNCTION:"..env.kind(srv)..":"..id)
				return false
			end
			env.check(srv,e) -- keep consistant
			if env.put(srv,e,t) then -- entity put ok
				if t.commit() then -- success
					env.cache_what(srv,e,mc) -- the new values
					env.cache_fix(srv,mc) -- change any memcached values we just adjusted
					return e -- return the adjusted entity
				end
			end
		else
			t.rollback()
			log("DATA UPDATE FAILED MISSING:"..env.kind(srv)..":"..id)
			return false
		end
		t.rollback() -- undo everything ready to try again
	end
	log("DATA UPDATE FAILED:"..env.kind(srv)..":"..id)
end


--------------------------------------------------------------------------------
--
-- this is like update, except entity will manifest if it does not exist
--
--------------------------------------------------------------------------------
function def_set(env,srv,id,f)

	if type(id)=="table" then id=id.key.id end -- can turn an entity into an id

	for retry=1,10 do
		local mc={}
		local t=wdata.begin()
		local e=env.get(srv,id,t) -- may or may not exist
		if not e then -- manifest
			e=env.create(srv,id)
		end

		env.cache_what(srv,e,mc) -- the original values
		if not e.key.notsaved then -- not a newly created entity
			if e.cache.updated>srv.time then -- stop any updates that time travel backwards
				t.rollback()
				log("DATA SET FAILED TIMETRAVEL:"..env.kind(srv)..":"..id)
				return false
			end
		end
		e.cache.updated=srv.time -- the function can change this change if it wishes
		if not f(srv,e) then -- hard fail
			t.rollback()
			log("DATA SET FAILED FUNCTION:"..env.kind(srv)..":"..id)
			return false
		end
		env.check(srv,e) -- keep consistant
		if env.put(srv,e,t) then -- entity put ok
			if t.commit() then -- success
				env.cache_what(srv,e,mc) -- the new values
				env.cache_fix(srv,mc) -- change any memcached values we just adjusted
				return e -- return the adjusted entity
			end
		end
		t.rollback() -- undo everything ready to try again
	end
	log("DATA SET FAILED:"..env.kind(srv)..":"..id)
	return false
end

--------------------------------------------------------------------------------
--
-- get or create but does not put
--
-- use the check function to fill with any special defaults
--
--------------------------------------------------------------------------------
function def_manifest(env,srv,id)

	if type(id)=="table" then id=id.key.id end -- can turn an entity into an id

	local e=env.get(srv,id) -- may or may not exist

	if e then
		return e
	else
		e=env.create(srv,id)
		return e
	end
	
end

--------------------------------------------------------------------------------
--
-- what key name should we use to cache an entity?
--
--------------------------------------------------------------------------------
function def_cache_key(env,srv,id)
	if type(id)=="table" then id=id.key.id end -- can turn an entity into an id
	return "ent="..env.kind(srv).."&id="..id
end


--------------------------------------------------------------------------------
--
-- given an entity return or update a list of memcache keys we should recalculate
-- this list is a name->bool lookup
--
--------------------------------------------------------------------------------
function def_cache_what(env,srv,ent,mc)
	local mc=mc or {} -- can supply your own result table for merges	
	
	local ck=env.cache_key(srv,ent.key.id)
	if ck then
		mc[ ck ] = true
	end
	return mc
end

--------------------------------------------------------------------------------
--
-- fix the memcache items previously produced by what_memcache
-- probably best just to delete them so they will automatically get rebuilt
-- but we could do more complicated things
--
--------------------------------------------------------------------------------
function def_cache_fix(env,srv,mc)
	for n,b in pairs(mc) do
		cache.del(srv,n)
	end
end



-----------------------------------------------------------------------------
--
-- set these default functions into the given environment
-- these are functions that handle basic data management
--
-- you must also provide
--
-- a kind function
-- a check function
-- a table of default cache values
-- a table of default props values
--
-----------------------------------------------------------------------------
function set_defs(env)

	env.create     = function(srv,id)     return def_create(env,srv,id)         end
	env.manifest   = function(srv,id)     return def_manifest(env,srv,id)       end
	env.put        = function(srv,ent,t)  return def_put(env,srv,ent,t)         end
	env.get        = function(srv,id,t)   return def_get(env,srv,id,t)          end
	env.set        = function(srv,id,f)   return def_set(env,srv,id,f)	 	    end
	env.update     = function(srv,id,f)   return def_update(env,srv,id,f)       end
	env.cache_key  = function(srv,id)     return def_cache_key(env,srv,id)      end
	env.cache_what = function(srv,ent,mc) return def_cache_what(env,srv,ent,mc) end
	env.cache_fix  = function(srv,mc)     return def_cache_fix(env,srv,mc)      end

	return env
end


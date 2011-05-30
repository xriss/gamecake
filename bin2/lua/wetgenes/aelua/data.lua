
local json=require("json")

local pairs=pairs
local require=require
local type=type

local os=os

local core=require("wetgenes.aelua.data.core")
local log=require("wetgenes.aelua.log").log

local tostring=tostring

module("wetgenes.aelua.data")
local cache=require("wetgenes.aelua.cache")
local dat=_M


function countzero()
	count=0
	api_time=0
end
countzero()

local kind_props={}	-- default global props mapped to kinds

local start_time -- handle simple api benchmarking of some calls
local function apis()
	start_time=os.time()
end
local function apie(...)
	api_time=api_time+os.time()-start_time
	return ...
end



function keyinfo(keystr)
	
	return core.keyinfo(keystr)
end

function keystr(kind,id,parent)

	return core.keystr(kind,id,parent)

end



function del(ent)
	apis()
	
	count=count+0.5
	
	return apie(core.del(nil,ent))
end

function put(ent)
	apis()
	count=count+0.5
	
	return apie(core.put(nil,ent))
end

function get(ent)
	apis()
	count=count+0.5
	return apie(core.get(nil,ent))
end

function query(q)
	apis()
	count=count+1
--log(tostring(q))	

	return apie(core.query(nil,q))
end

-----------------------------------------------------------------------------
--
-- Begin a transaction, use the functions inside the returned table
-- to perform actions within this transaction
--
-- the basic code flow is that you should begin one transaction per entity(parent)
-- and then rollback all when one fails. the first del/put/get locks the entity
-- we are dealing with in this transaction
--
-- after the t.fail flag gets set on a put/del then everything apart from rollback just returns nil
-- and commit is turned into an auto rollback
--
-- so this is OK transaction code, just remember that puts may not auto generate a key
-- and there may be other reasons for fails
--
-- for _=1,10 do -- try a few times
--     t=begin()
--     if t.get(e) then e.props.data=e.props.data.."new data" end
--     t.put(e)
--     if t.commit() then break end -- success
-- end
--
-----------------------------------------------------------------------------
function begin()

	local t={}
	t.core=core.begin()
	
	t.fail=false -- this will be set to true when a transaction action fails and you should rollback and retry
	t.done=false -- set to true on commit or rollback to disable all methods
	
 -- these methods are the same as the global ones but operate on this transaction
 	t.del=function(ent)	if t.fail or t.done then return nil end apis() return apie(core.del(t,ent)) end
	t.put=function(ent)	if t.fail or t.done then return nil end apis() return apie(core.put(t,ent)) end
	t.get=function(ent)	if t.fail or t.done then return nil end apis() return apie(core.get(t,ent)) end
	t.query=function(q)	if t.fail or t.done then return nil end apis() return apie(core.query(t,q)) end
	
	t.rollback=function() -- returns false to imply that nothing was commited
		if t.done then return false end -- safe to rollback repeatedly
		t.done=true
		apis()
		t.fail=not apie(core.rollback(t.core)) -- we always set fail and return false
		return not t.fail
	end	
	
	t.commit=function() -- returns true if commited, false if not
		if t.done then return false end -- safe to rollback repeatedly
		if t.fail then -- rollback rather than commit
			apis()
			return apie(t.rollback())
		end
		t.done=true
		apis()
		t.fail=not apie(core.commit(t.core))
		return not t.fail
	end

	return t
end


-----------------------------------------------------------------------------
--
-- build cache which is a mixture of decoded json vars (this may contain sub tables)
-- overiden by database props which do not contain tables but are mildly searchable
-- props.json should contain this json data string on input
-- cache will be a filled in table to be used instead of props
--
-- Not sure if this is more compact than just creating many real key/value pairs
-- but it feels like a better way to organize. :)
--
-- At least it is a bit more implicit about what can and cannot be searched for.
--
-- the idea is everything we need is copied into the cache, you can edit it there
-- and then build_props will do the reverse in preperation for a put
--
-----------------------------------------------------------------------------
function build_cache(e)

	if e.props.json then -- expand the json data
	
		e.cache=json.decode(e.props.json)
		
	else
	
		e.cache={}
	
	end

	for i,v in pairs(e.props) do -- override cache by props
		e.cache[i]=v
	end
	
	e.cache.json=nil -- not the json prop
	
	if e.key then -- copy the key data
		e.cache.parent=e.key.parent
		e.cache.kind=e.key.kind
		e.cache.id=e.key.id
	end
	
	return e
end
-----------------------------------------------------------------------------
--
-- a simplistic reverse of build cache
-- any props of the same name will get updated from this cache
-- rather than encoded into props.json
--
-----------------------------------------------------------------------------
function build_props(e)

	local t={}
	local ignore={kind=true,id=true,parent=true,json=true,} -- special names to ignore
	
	for i,v in pairs(e.cache) do
		if ignore[i] then -- ignore these special names
		elseif e.props[i] then
			e.props[i]=v -- if it exists as a prop then the prop is updated
		else
			t[i]=v -- else it just goes into the json prop
		end
	end
	e.props.json=json.encode(t)
	
	return e
end





--------------------------------------------------------------------------------
--
-- Create a new local entity filled with initial data
--
--------------------------------------------------------------------------------
function def_create(env,srv)

	local ent={}
	
	ent.key={kind=env.kind(srv)} -- we will not know the key id until after we save
	ent.props={}
	
	local p=ent.props
	
	p.created=srv.time
	p.updated=srv.time
	
	for i,v in pairs(env.default_props or {}) do
		p[i]=v
	end

	dat.build_cache(ent) -- this just copies the props across
	
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

	t=tt or dat -- use transaction?

	local _,ok=env.check(srv,ent) -- check that this is valid to put
	if not ok then return nil end

	dat.build_props(ent)
	local ks=t.put(ent)
	
	if ks then
		ent.key=dat.keyinfo( ks ) -- update key with new id
		dat.build_cache(ent)
		
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
	if not tt then -- can try for cached value outside of transactions
		local ent=cache.get(srv,ck)
		if ent then return env.check(srv,ent) end -- Yay, we got a cached value
	end
	
	local t=tt or dat -- use transaction?
	
	if not t.get(ent) then return nil end	
	dat.build_cache(ent)
	
	if not tt then -- auto cache ent for one hour
		cache.put(srv,ck,ent,60*60)
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
		local t=dat.begin()
		local e=env.get(srv,id,t)
		if e then
			env.cache_what(srv,e,mc) -- the original values
			if e.props.created~=srv.time then -- not a newly created entity
				if e.cache.updated>=srv.time then t.rollback() return false end -- stop any updates that time travel
			end
			e.cache.updated=srv.time -- the next function can change this change if it wishes
			if not f(srv,e) then t.rollback() return false end -- hard fail
			env.check(srv,e) -- keep consistant
			if env.put(srv,e,t) then -- entity put ok
				if t.commit() then -- success
					env.cache_what(srv,e,mc) -- the new values
					env.cache_fix(srv,mc) -- change any memcached values we just adjusted
					return e -- return the adjusted entity
				end
			end
		end
		t.rollback() -- undo everything ready to try again
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
	
	mc[ env.cache_key(srv,ent.key.id) ] = true
	
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
-- you may also want to provide
--
-- a manifest function to auto create a given id
--
-- see dumid.users for an example of how to do this
--
-----------------------------------------------------------------------------
function set_defs(env)

	env.create     = function(srv)        return def_create(env,srv)            end
	env.put        = function(srv,ent,t)  return def_put(env,srv,ent,t)         end
	env.get        = function(srv,id,t)   return def_get(env,srv,id,t)          end
	env.update     = function(srv,id,f)   return def_update(env,srv,id,f)       end
	env.cache_key  = function(srv,id)     return def_cache_key(env,srv,id)      end
	env.cache_what = function(srv,ent,mc) return def_cache_what(env,srv,ent,mc) end
	env.cache_fix  = function(srv,mc)     return def_cache_fix(env,srv,mc)      end

	return env
end



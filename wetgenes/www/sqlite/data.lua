-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log


local sql=require("sqlite")
local wstr=require("wetgenes.string")
local wsql=require("wetgenes.www.sqlite")


local fixvalue=wsql.fixvalue

module(...)
local wdata=require(...) -- this is us
local cache=require("wetgenes.www.any.cache")


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

local prefix="sqlite/"
local postfix=".sqlite"

local function fixkind(kind) return kind:gsub("%p","_") end

function getdb(kind)

-- at this point we can choose to return a single database no matter what the kind
-- or open a seperate one for each kind which might make more sense

	db=wsql.open(wsql.dbs,prefix,fixkind(kind),postfix)
	return db
end

function keyinfo(keystr)
--	log("data.keyinfo:")
	
	local t=wstr.split(keystr,"/")
	
	return {kind=t[1],id=tonumber(t[2])}
	
--	return core.keyinfo(keystr)
end

function keystr(kind,id,parent)
--	log("data.keystr:")
	
	return kind.."/"..id

--	return core.keystr(kind,id,parent)

end



function del(ent,t)
	local kind=fixkind(ent and ent.key and ent.key.kind)
	local id=ent and ent.key and ent.key.id
	local ret
--	log(wstr.serialize(ent))
	log("data.del:",kind)
	apis()
	
	count=count+0.5
	
	local db=getdb(kind)

	local s="DELETE FROM "..kind.." WHERE id="..fixvalue(id)..";\n"
	
log(s)
	ret=wsql.exec(db,s)
	apie()
	return ret
end

function put(ent,t)
	local kind=fixkind(ent and ent.key and ent.key.kind)
	local id=ent and ent.key and ent.key.id
	local ret
--	log(wstr.serialize(ent))
	log("data.put:",kind)
	apis()
	count=count+0.5

	local db=getdb(kind)
	
	local s=make_replace(kind,ent.props)
	
log(s)
	ret=wsql.exec(db,s)
	apie()
	return ret
end

function get(ent,t)
	local kind=fixkind(ent and ent.key and ent.key.kind)
	local id=ent and ent.key and ent.key.id
	local ret
--	log(wstr.serialize(ent))
	log("data.get:",kind)
	apis()
	count=count+0.5

	local db=getdb(kind)

	local s="SELECT * FROM "..kind.." WHERE id="..fixvalue(id)..";\n"

log(s)
	ret=wsql.row(db,s)
	
	apie()
	return ret
end

function query(q,t)
	local kind=fixkind(ent and ent.key and ent.key.kind)
	local ret
	log("data.query:")
	apis()
	count=count+1
--log(tostring(q))	

	local db=getdb(kind)

	apie()
	return ret
end

function rollback(t)
end
function commit(t)
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
	log("data.begin:")

	local t={}
--	t.core=core.begin()
	
	t.fail=false -- this will be set to true when a transaction action fails and you should rollback and retry
	t.done=false -- set to true on commit or rollback to disable all methods
	
 -- these methods are the same as the global ones but operate on this transaction
 	t.del=function(ent)	if t.fail or t.done then return nil end return del(ent,t) end
	t.put=function(ent)	if t.fail or t.done then return nil end return put(ent,t) end
	t.get=function(ent)	if t.fail or t.done then return nil end return get(ent,t) end
	t.query=function(q)	if t.fail or t.done then return nil end return query(q,t) end
	
	t.rollback=function() -- returns false to imply that nothing was commited
		if t.done then return false end -- safe to rollback repeatedly
		t.done=true
		t.fail=not rollback(t) -- we always set fail and return false
		return not t.fail
	end	
	
	t.commit=function() -- returns true if commited, false if not
		if t.done then return false end -- safe to rollback repeatedly
		if t.fail then -- rollback rather than commit
			return rollback(t)
		end
		t.done=true
		apis()
		t.fail=not commit(t)
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
	
	p.created=srv and srv.time or os.time() -- allow srv to be nil
	p.updated=srv and srv.time or p.created
	
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
	log("data.def_put:")
	
	t=tt or wdata -- use transaction?

	local _,ok=env.check(srv,ent) -- check that this is valid to put
	if not ok then return nil end

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
	log("data.def_get:")
	
	if not id then return nil end

	local ent=id
	
	if type(ent)~="table" then -- get by id
		ent=env.create(srv)
		ent.key.id=id
	end
	
	local ck=env.cache_key(srv,ent.key.id)
	if not tt then -- can try for cached value outside of transactions
		local ent=cache.get(srv,ck)
		if ent then return env.check(srv,json.decode(ent)) end -- Yay, we got a cached value
	end
	
	local t=tt or wdata -- use transaction?
	
	if not t.get(ent) then return nil end	
	wdata.build_cache(ent)
	
	if not tt then -- auto cache ent for one hour
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
	log("data.def_update:")

	if type(id)=="table" then id=id.key.id end -- can turn an entity into an id
		
	for retry=1,10 do
		local mc={}
		local t=wdata.begin()
		local e=env.get(srv,id,t) -- must exist
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
-- get or create then fill and put, similar to update but will create new data
--
-- f must be a function that fills the entity and returns true on success
--
-- id can be an id or an entity from which we will get the id
--
--------------------------------------------------------------------------------
function def_manifest(env,srv,id,f)
	log("data.def_manifest:")

	if type(id)=="table" then id=id.key.id end -- can turn an entity into an id
		
	for retry=1,10 do
		local mc={}
		local t=wdata.begin()
		local e=env.get(srv,id,t) -- may or may not exist
		if e then
			env.cache_what(srv,e,mc) -- the original values
			if e.cache.updated>=srv.time then t.rollback() return false end -- stop any updates that time travel
			e.cache.updated=srv.time -- the next function can change this change if it wishes
			if not f(srv,e) then t.rollback() return e end -- just return orig
		else
			e=env.create(srv)
			e.key.id=id
			e.cache.id=id
			if not f(srv,e) then t.rollback() return false end -- hard fail
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
-----------------------------------------------------------------------------
function set_defs(env)

	env.create     = function(srv)        return def_create(env,srv)            end
	env.put        = function(srv,ent,t)  return def_put(env,srv,ent,t)         end
	env.get        = function(srv,id,t)   return def_get(env,srv,id,t)          end
	env.update     = function(srv,id,f)   return def_update(env,srv,id,f)       end
	env.manifest   = function(srv,id,f)   return def_manifest(env,srv,id,f)     end
	env.cache_key  = function(srv,id)     return def_cache_key(env,srv,id)      end
	env.cache_what = function(srv,ent,mc) return def_cache_what(env,srv,ent,mc) end
	env.cache_fix  = function(srv,mc)     return def_cache_fix(env,srv,mc)      end

	return env
end

function setup_db(env,srv)

-- make sure database exists and is setup

	local kind=env.kind()

	log("data.setup_db:",kind)


	local db=getdb(kind)

-- all data has these fields	
	local info={
		{name="id",INTEGER=true,PRIMARY=true},
		{name="created",INTEGER=true},
		{name="updated",INTEGER=true},
		{name="json",TEXT=true},
	}
	
--check if is already added
	local function in_table(tab,name)
		for i,v in ipairs(tab) do
			if v.name==name then return true end
		end
	end
	
		
	for n,v in pairs(env.default_props) do
		if not in_table(info,n) then -- add to table, simple check for dupes just in case.
			local t={name=n}
			if tp=="number" then -- only have numbers or strings and numbers are real
				t.REAL=true
			else
				t.TEXT=true
			end
			info[#info+1]=t
		end
	end
	

-- check or update database
	wsql.set_info(db,kind,info)
	
end

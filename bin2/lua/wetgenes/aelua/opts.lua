
local dat=require("wetgenes.aelua.data")
local cache=require("wetgenes.aelua.cache")

local math=math
local string=string
local table=table
local os=os

local ipairs=ipairs
local pairs=pairs
local tostring=tostring
local tonumber=tonumber
local type=type
local pcall=pcall
local loadstring=loadstring


-- a place to keep options such as passwords that
-- the server needs to know about but are different per app
-- and obviously should not be included in code

-- data is kept in the datastore and also cached in the memcache (todo)

module("wetgenes.aelua.opts")

--------------------------------------------------------------------------------
--
-- serving flavour can be used to create a subgame of a different flavour
-- make sure we incorporate flavour into the name of our stored data types
--
--------------------------------------------------------------------------------
function kind()
	return "opts"
end

--------------------------------------------------------------------------------
--
-- Create a new local entity filled with initial data
--
--------------------------------------------------------------------------------
function create()

	local t=os.time()

	local ent={}
	
	ent.key={kind=kind()} -- we will not know the key id until after we save
	ent.props={}
	
	local p=ent.props
	
	p.created=t
	p.updated=t
		
	dat.build_cache(ent) -- this just copies the props across
	
-- these are json only vars
	local c=ent.cache

	return check(ent)
end

--------------------------------------------------------------------------------
--
-- check that entity has initial data and set any missing defaults
-- the second return value is false if this is not a valid entity
--
--------------------------------------------------------------------------------
function check(ent)

	local ok=true

	local c=ent.cache
		
	return ent,ok
end

--------------------------------------------------------------------------------
--
-- Save to database
-- this calls check before putting and does not put if check says it is invalid
-- build_props is called so code should always be updating the cache values
--
--------------------------------------------------------------------------------
function put(ent,t)

	t=t or dat -- use transaction?

	local _,ok=check(ent) -- check that this is valid to put
	if not ok then return nil end

	dat.build_props(ent)
	local ks=t.put(ent)
	
	if ks then
		ent.key=dat.keyinfo( ks ) -- update key with new id
		dat.build_cache(ent)
	end

	return ks -- return the keystring which is an absolute name
end


--------------------------------------------------------------------------------
--
-- Load from database, pass in id or entity
-- the props will be copied into the cache
--
--------------------------------------------------------------------------------
function get(id,t)

	local ent=id
	
	if type(ent)~="table" then -- get by id
		ent=create()
		ent.key.id=id
	end
	
	t=t or dat -- use transaction?
	
	if not t.get(ent) then return nil end	
	dat.build_cache(ent)
	
	return check(ent)
end


--------------------------------------------------------------------------------
--
-- read a string
--
--------------------------------------------------------------------------------
function get_dat(id)
	local e=get(id,t)
	if e then return e.cache.dat end
	return nil
end
--------------------------------------------------------------------------------
--
-- write a string
--
--------------------------------------------------------------------------------
function put_dat(id,dat)
	local e=create()
	e.key.id=id
	e.cache.dat=dat
	local r=put(e)
	
	return r
end

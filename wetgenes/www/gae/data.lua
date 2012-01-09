-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local json=require("wetgenes.json")

local core=require("wetgenes.www.gae.data.core")
local log=require("wetgenes.www.any.log").log

local wstr=require("wetgenes.string")

module(...)
local wdata=require(...) -- this is us
package.loaded["wetgenes.www.any.data"]=wdata
local cache=require("wetgenes.www.any.cache")
local wdatadef=require("wetgenes.www.any.datadef")


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



function set_defs(env)
	return wdatadef.set_defs(env)
end

--no need for this, so just a stub
function setup_db(env,srv)
end



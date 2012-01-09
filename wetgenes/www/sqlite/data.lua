-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log


local sql=require("sqlite")
local wstr=require("wetgenes.string")
local wsql=require("wetgenes.www.sqlite")

local json=require("wetgenes.json")

local fixvalue=wsql.fixvalue

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

local prefix="sqlite/"
local postfix=".sqlite"

local function fixkind(kind) return kind:gsub("%p","_") end

function getdb(kind)

-- at this point we can choose to return a single database no matter what the kind
-- or open a seperate one for each kind which might make more sense

--	db=wsql.open(wsql.dbs,prefix,fixkind(kind),postfix)

	db=wsql.open(wsql.dbs,prefix,"data",postfix)
	return db
end

function keyinfo(keystr)
	local t=wstr.split(keystr,"/")	
	return {kind=t[1],id=tonumber(t[2]) or t[2] }
end

function keystr(kind,id,parent)
	return kind.."/"..id
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

	local s
	if type(id)=="number" then
		s="DELETE FROM "..kind.." WHERE ROWID="..fixvalue(id)..";"
	else
		s="DELETE FROM "..kind.." WHERE id="..fixvalue(id)..";"
	end
log(s)
	ret=wsql.exec(db,s)
	apie()
	return true
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
	
	local s=wsql.make_replace(kind,ent.props)
	
log(s)
	ret=wsql.exec(db,s)
	id=id or db:last_insert_rowid() -- get the new id, unless we forced it
	
	ent.key.id=id -- fix id
	
	apie()
	return keystr(kind,id)
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

	local s
	if type(id)=="number" then
		s="SELECT * FROM "..kind.." WHERE ROWID="..fixvalue(id)..";"
	else
		s="SELECT * FROM "..kind.." WHERE id="..fixvalue(id)..";"
	end

log(s)
	ent.props=wsql.row(db,s)
	
if ent.props then
--	log(wstr.serialize(ent.props))
end

	apie()
	return ent.props and ent
end

function query(q)
	local kind=fixkind(q and q.kind)
	local ret
	log("data.query:")
	apis()
	count=count+1
--log(tostring(q))	

	local db=getdb(kind)

	apie()
	return {list={}}
end

function rollback(t)
end
function commit(t)
	return true
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
		e.cache.id=e.props.id or e.key.id -- use string or ROWID?
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

	if e.key.id then
		if type(e.key.id)=="number" then
			e.props.ROWID=e.key.id -- use number id from key
		else
			e.props.id=e.key.id -- use string id from key
		end
	end
	
	return e
	
end


function set_defs(env)
	return wdatadef.set_defs(env)
end


function setup_db(env,srv)

-- make sure database exists and is setup

	local kind=env.kind()

	log("data.setup_db:",kind)


	local db=getdb(kind)

-- all data has these fields	
	local info={
		{name="id",TEXT=true,UNIQUE=true},
		{name="created",REAL=true},
		{name="updated",REAL=true},
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

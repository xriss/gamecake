
-- handle mostly opaque data stored in a mysql database
-- json is the easy/suported data format across various "web" languages i use ( as2/as3/jscript/lua/php )
-- so the data is serialised in json format

local tonumber=tonumber


local sql=require("wetgenes.cgilua.mysql")
local json=require("json")


local cgi = wetgenes.cgilua or require("wetgenes.cgilua")
local cfg = cfg


local cgilua=cgilua

local os=os



local dbg=dbg or function(s) cgilua.errorlog(s) end



module("wetgenes.cgilua.mysqljson")


base_sqltable={
	--	name		type			NULL	KEY,	default		extra
	{	"id",		"bigint(20)",	"NO",	"PRI",	nil,		"auto_increment"	}, -- unique object ID
	{	"last",		"bigint(20)",	"NO",	"MUL",	nil,		nil					}, -- last updated time
	{	"latch",	"int(11)",		"NO",	"MUL",	nil,		nil					}, -- a "lock" counter
	{	"json",		"text",			"NO",	nil,	nil,		nil					}, -- serialised data
}


-----------------------------------------------------------------------------
--
-- create a data table
--
-----------------------------------------------------------------------------
function create_table(tabname)

	sql.table_create(tabname,base_sqltable)

end


-----------------------------------------------------------------------------
--
-- create a new item, you will need to call put, before it is actually created in the database
-- and of course the put may fail
-- set the id to 0 if you want to get an auto inc id
-- you may also create an item with a user id and perform a get rather than a put
--
-----------------------------------------------------------------------------
function create(tabname,id)

local it={}

	it.tabname=tabname
	it.id=id or 0
	it.last=os.time()
	it.latch=0
	it.data={}
	it.dirty=false
	it.json=json.Encode(it.data)
	
	return it

end

-----------------------------------------------------------------------------
--
-- write data to database
-- this function may FAIL :)
-- need to take apropriate action if it does
--
-----------------------------------------------------------------------------
function put(it)

	it.json=json.Encode(it.data) -- encode data, ready for send
	
	if it.id==0 or it.latch==0 then -- first write, so create a new ID
	
		local tab={}
		local t=os.time()
		local l=1
		
		tab.last=t
		tab.latch=l
		tab.json=it.json
		
		if it.id==0 then -- do not care about id
		
			local ret=sql.do_insert(it.tabname,tab)
			
			if ret==0 then return false end
			
			it.id=ret
		
		else -- we wish to create a fixed id
		
			tab.id=it.id
			local ret=sql.do_insert(it.tabname,tab)
			
			if ret~=1 then return false end
		end
	
		return true
			
	else -- update an existing item
	
		local tab={}
		local t=os.time()
		local l=it.latch+1
		
		tab.last=t
		tab.latch=l
		tab.json=it.json
		
		local ret=sql.do_update(it.tabname,tab," id="..it.id.." AND latch="..it.latch)
		
		if ret==1 then -- updated OK
		
			it.latch=l
			it.last=t
		
			return true
		else
		
			it.dirty=true -- flag data as dirty
		
			return false
		end
		
	end
	
	return false

end

-----------------------------------------------------------------------------
--
-- get data from database using id
--
-----------------------------------------------------------------------------
function get(it)

	local res=sql.execute([[
			SELECT *
			FROM ]]..it.tabname..[[
			WHERE id=]]..it.id..[[ ]])
			
	local tab=sql.named(res,1)
	
	if not tab then return false end
	
	it.last=tonumber(tab.last)
	it.latch=tonumber(tab.latch)
	it.json=tab.json
	
	it.data=json.Decode(it.json) or {} -- decode data so we may edit it
	
	it.dirty=false
	
	return true

end


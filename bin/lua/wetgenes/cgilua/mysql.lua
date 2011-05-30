
--
-- Mysql helper functions
--


local type,ipairs,pairs,tonumber=type,ipairs,pairs,tonumber

local string=string

local cgilua=cgilua

local cfg = cfg


local luasql = require("luasql.mysql")
local sql = luasql.mysql()

local dbg=dbg or function(s) cgilua.errorlog(s) end

module("wetgenes.cgilua.mysql")

query_count=0


-----------------------------------------------------------------------------
--
-----------------------------------------------------------------------------
function setup()
	clean()
	
local err

	con,err=sql:connect(cfg.mysql_database,cfg.mysql_username,cfg.mysql_password,cfg.mysql_hostname)
	
	if not con then
	
		dbg("\n".."Failed to connect to mysql"..(err or "unknown").."\n") -- print error
		
		return nil
	end
	
	cgilua.addclosefunction( clean )

	return _M
end
function clean()
	if con then con:close() end
	con=nil
end


-----------------------------------------------------------------------------
--
-- convert a string into a hex string
--
-----------------------------------------------------------------------------
function str_to_hex(s)
	return string.gsub(s, ".", function (c)
		return string.format("%02x", string.byte(c))
	end)
end

-----------------------------------------------------------------------------
--
-- escape a string for mysql, convert to a (possibly large) number in hex
--
-----------------------------------------------------------------------------
function mysql_escape(s)
	return "0x"..str_to_hex(s)
end



--[[
+------------+----------+------+-----+---------+----------------+
| Field      | Type     | Null | Key | Default | Extra          |
+------------+----------+------+-----+---------+----------------+
| Id         | int(11)  | NO   | PRI | NULL    | auto_increment |
| Name       | char(35) | NO   |     |         |                |
| Country    | char(3)  | NO   | UNI |         |                |
| District   | char(20) | YES  | MUL |         |                |
| Population | int(11)  | NO   |     | 0       |                |
+------------+----------+------+-----+---------+----------------+
]]

-----------------------------------------------------------------------------
--
-- mysql table creation system
--
-- check if a table exists, if it doesnt then just create it
--
-- then check each column, any column that doesnt exist but should is then added
--
-- name is the name of the table
-- cols, consists of the result you would expect from a SHOW COLUMNS FROM query
-- any descrepencies will be reported,
-- see above for example showing columns output
--
-- extra is other mysql key creation to put onto the end of the initial table creation
-- any post creation optimisation will have to happen by hand, maintaining the table columns consistency is the important bit
--
-----------------------------------------------------------------------------
function table_create(name,cols,extra)

local cur,err
local q

-- build table creation string

	q=""
	q=q..[[ CREATE TABLE IF NOT EXISTS `]]..name..[[` ( ]].."\n"
	
	for i,v in ipairs(cols) do
	
		local l
		
		if i==1 then l="" else l=",\n" end
		
		l=l..[[ `]]..v[1]..[[` ]]..v[2]
		
		if v[3]=="NO" then l=l.." NOT NULL" end
		if v[5] then l=l.." DEFAULT "..v[5] end
		if v[6] then l=l.." "..v[6] end
		
		q=q..l
	
	end
		
	for i,v in ipairs(cols) do -- add extra key optimizations to the table
	
		if v[4] then
		
			local l=",\n"
			
			if v[4]=="PRI" then
			
				l=l..[[ PRIMARY KEY (`]]..v[1]..[[`) ]]
			
			elseif v[4]=="MUL" then
			
				l=l..[[ KEY `]]..v[1]..[[` (`]]..v[1]..[[`) ]]
				
			elseif v[4]=="UNI" then
			
				l=l..[[ UNIQUE KEY `]]..v[1]..[[` (`]]..v[1]..[[`) ]]
			
			end
			
			q=q..l
			
		end
	
	end
	
	if extra then -- include extra table creation lines for keys etc
	
		q=q..",\n "..extra
	
	end
		
	q=q..[[ ) ]].."\n"

--dbg("**Preparing table " ..name.."\n")
--dbg(q)

	cur,err=con:execute(q)
	if not cur and err then dbg("\n"..err.."\n") end
	
	cur,err=con:execute("SHOW COLUMNS FROM "..name)
	if not cur and err then dbg("\n"..err.."\n") end
	
	local i=1
	local r
	local tab={}

	repeat
	
		r=cur:fetch({})
		tab[i]=r
		i=i+1
	
	until not r
	
--dbg("**Checking table " ..name.."\n")
	for i,v in ipairs(tab) do
	
		local s
		for ii,vv in ipairs(v) do
			if s then s=s.."\t"..vv else s=vv end
		end
	
--		dbg( s.."\n" )
	
	end
	
--dbg("**Created table " ..name.."\n")

	if cur then cur:close() end
	
end


-----------------------------------------------------------------------------
--
-- perform the sql queery
--
-----------------------------------------------------------------------------
function execute(q,flags)

local cur,ret,err
local lastid,lastcur

	
query_count=query_count+1
	cur,err=con:execute(q)
	
	if not cur and err then -- indicate failure
	
		dbg("\n"..err.."\n") -- print error
		
		return nil
	end
	
	if type(cur)=="number" then -- a single number return value
	
		if flags=="lastid" and (cur~=0) then -- get last id
		
			lastcur,err=con:execute("SELECT LAST_INSERT_ID()")
			
			if not lastcur and err then -- indicate failure
			
				dbg("\n"..err.."\n") -- print error
				
				lastid=0
				
			else
			
				lastid=lastcur:fetch({})[1]
				
				lastid=tonumber(lastid)
				
				if lastcur then lastcur:close() end
				
--				dbg("\n LASTID = "..lastid.."\n") -- print error
			end
			
			return lastid
		end
	
		return cur
	end
	
	ret={}
	
	ret.names=cur:getcolnames()
	ret.types=cur:getcoltypes()
	
	local i=1
	local r
	
	repeat
	
		r=cur:fetch({})
		ret[i]=r
		i=i+1
	
	until not r
		
	if cur then cur:close() end
	return ret
end


-----------------------------------------------------------------------------
--
-- fill up a table with named results
--
-----------------------------------------------------------------------------
function named(tab,idx)

	if (not tab) or (not tab[idx]) then return nil end

local ret={}

	for i,v in ipairs(tab.names) do
		ret[v]=tab[idx][i]
	end

	return ret
end


-----------------------------------------------------------------------------
--
-- insert or update data on clash, similar format to the lanes returned info
-- this function doesnt do anything it just builds a queery string that will
--
-- name == table name
-- row == data to insert
--
-- row.names == table of named columns
-- row[1] == table of data (strings need to have already been fixed/escaped)
--
-- this is a single insert or update, so there is only one row
--
-----------------------------------------------------------------------------
function make_set(name,row)

local q="INSERT INTO "..name.." SET "

	for i,v in ipairs(row.names) do
	
		local sep=", \n" if i==1 then sep=" \n" end
		
		q=q..sep..v.."="..row[1][i].." "
	
	end
	
	q=q.."\n ON DUPLICATE KEY UPDATE "

	for i,v in ipairs(row.names) do
	
		local sep=", \n" if i==1 then sep=" \n" end
		
		q=q..sep..v.."="..row[1][i].." "
	
	end
		
	return q
end

-----------------------------------------------------------------------------
--
-- insert only, similar format to the lanes returned info
-- this function doesnt do anything it just builds a queery string that will
--
-- name == table name
-- row == data to insert
--
-- row.names == table of named columns
-- row[1] == table of data (strings need to have already been fixed/escaped)
--
-- this is a single insert or update, so there is only one row
--
-----------------------------------------------------------------------------
function make_insert(name,row)

local q="INSERT INTO "..name.." SET "

	for i,v in ipairs(row.names) do
	
		local sep=", \n" if i==1 then sep=" \n" end
		
		q=q..sep..v.."="..row[1][i].." "
	
	end
	
	q=q.."\n"
		
	return q
end

-----------------------------------------------------------------------------
--
-- update only, similar format to the lanes returned info
-- this function doesnt do anything it just builds a queery string that will
--
-- name == table name
-- row == data to insert
-- where == where to update
--
-- row.names == table of named columns
-- row[1] == table of data (strings need to have already been fixed/escaped)
--
-- this is a single insert or update, so there is only one row
--
-----------------------------------------------------------------------------
function make_update(name,row,where)

local q="UPDATE "..name.." SET "

	for i,v in ipairs(row.names) do
	
		local sep=", \n" if i==1 then sep=" \n" end
		
		q=q..sep..v.."="..row[1][i].." "
	
	end
	
	q=q.."\n WHERE "..where
		
	return q
end

-----------------------------------------------------------------------------
--
-- convert a table into a row set
--
-- returns row where
--
-- row.names == table of named columns
-- row[1] == table of data with strings fixed/escaped
--
-----------------------------------------------------------------------------
function tab_to_row(dat)

	local idx=1
	local row={}
	row.names={}
	row[1]={}
	
	for n,v in pairs(dat) do -- build and fixup mysql data
	
		row.names[idx]=n
		
		if type(v)=="string" then
		
			if v=="" then
			
				row[1][idx]="DEFAULT"
				
			else
		
				row[1][idx]=mysql_escape(v)
			
			end
			
		else
		
			row[1][idx]=v
		
		end
		
		idx=idx+1
	
	end
		
	return row
end

-----------------------------------------------------------------------------
--
-- create and perform a blocking data insert or update
--
-- just pass in a tab and it will build and execute a sql query to add or change that data
--
-----------------------------------------------------------------------------
function do_set(name,tab)

	return execute( make_set(name,tab_to_row(tab)) , "lastid" )
	
end

-----------------------------------------------------------------------------
--
-- create and perform a blocking data insert
--
-- just pass in a tab and it will build and execute a sql query to add or change that data
--
-----------------------------------------------------------------------------
function do_insert(name,tab)

	return execute( make_insert(name,tab_to_row(tab)) , "lastid" )
	
end

-----------------------------------------------------------------------------
--
-- create and perform a blocking data update
--
-- just pass in a tab and it will build and execute a sql query to add or change that data
--
-----------------------------------------------------------------------------
function do_update(name,tab,where)

	return execute( make_update(name,tab_to_row(tab),where) )
	
end

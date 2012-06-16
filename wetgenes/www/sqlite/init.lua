-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=print

if ngx then
	log=require("wetgenes.www.any.log").log
end

local sql=require("sqlite")
local wstr=require("wetgenes.string")

local function fixkind(kind) return kind:gsub("%p","_") end

module(...)

dbs={} -- tooglobal?pass in this or your own to the funcs anyhow

function open(dbs,prefix,kind,postfix) -- multiple opens are ok and get you the same db

	kind=fixkind(kind)

	local db=dbs[kind]
	
	if db then return db end -- already open
	
	db=assert(sql.open(prefix..kind..postfix))

	set_pragmas(db) -- always run this


	dbs[kind]=db -- remember
	return db
	
end

function close(dbs,kind)

	kind=fixkind(kind)

	local db=dbs[kind]
	
	if db then
		sb:close()
		dbs[kind]=nil
	end
end

-- setup pragmas, should run this after opening a db
function set_pragmas(db)
exec(db,[[
PRAGMA synchronous = 0 ;
PRAGMA encoding = "UTF-8" ;
]])

-- tunrning sync off is dangerous, but so is life
-- technically its only dangerous if you lose power whilst writing to disk
-- this is perhaps less likely than file coruption on a failing disk

end

-- wrap db:exec with an error checker
function exec(db,s,f,d)
--log(s)
	if 	db:exec(s,f,d)~=sql.OK then error(db:errmsg()) end
end

-- get all rows the sql requests
function rows(db,s)

	local d={}
--	local f=function(d,count,v,n)
--		local dd={}
--		d[#d+1]=dd
--		for i=1,count do dd[ n[i] ]=v[i] end
--		return 0
--	end
	
	for r in db:nrows(s) do
		d[#d+1]=r
	end

--	if 	db:exec(s,f,d)~=sql.OK then error(db:errmsg()) end

	return d
end

-- get first row the sql requests
function row(db,s)
	return rows(db,s)[1]
end

-- get info about a table, this can only work if WE created the table
function get_info(db,kind)

	kind=fixkind(kind)

--[[
	local d=rows(db,"PRAGMA table_info('"..name.."')");
	print(wstr.serialize(d))
]]

	local d=rows(db,"select sql from sqlite_master where name = '"..kind.."';")
	
	if not d[1] then return end -- no table of the given kind exists
	
-- grab the bit in brackets
	local _,_,s=string.find(d[1].sql,"%((.*)%)")
--print(s)
-- and split it by commas
	local a=wstr.split(s,",")
	
	tab={}
	
	local flags={"NOT","NULL","INTEGER","REAL","TEXT","BLOB","PRIMARY","FOREIGN","KEY","COLLATE","BINARY","NOCASE","RTRIM","UNIQUE","CHECK","DEFAULT"}
	for i,v in ipairs(flags) do flags[v]=0 end -- set as this next word
	flags.DEFAULT=1 -- set as the next word
	
	for i,v in ipairs(a) do
		local c=wstr.split_words(v)
--		print(wstr.serialize(c))
		local d={}
		for i,v in ipairs(c) do d[v]=flags[v] and c[i+flags[v]] end -- set flags only if we recognise them
		local cmd=false
		for i,v in ipairs(flags) do if c[1]:sub(1,#v)==v then cmd=v end end
		if cmd then
			d.cmd=c[1] -- set the command
		else -- a named column
			d.name=c[1] -- set the name
			if d.name:sub(1,1)=="'" then d.name=d.name:sub(2,-2) end -- strip quotes
		end

		tab[i]=d
	end
	
--	print(wstr.serialize(tab))
	return tab
end

-- create or update a table, this can only update if *we* created the table using this function
-- info is the same as when returned from info function
-- the two arecompared and the table updated with any missing columns
-- so you may not get a a table in the exact order specified or it may have extra cruft etc
--
-- in general it should be safe to add columns to the end of the info and call this again
-- so we can modify existing tabs
function set_info(db,kind,info)

	kind=fixkind(kind)

--	print(wstr.serialize(info))

	old=get_info(db,kind)

-- build the sql string we need to run	
	local t={}
	local p=function(...) for i,v in ipairs{...} do t[#t+1]=tostring(v) end end

-- add a column
	local function pdef(t)
		if t.name then
			p("'"..t.name.."'")
			if t.INTEGER then
				p(" INTEGER")
			elseif t.REAL then
				p(" REAL")
			elseif t.TEXT then
				p(" TEXT")
			elseif t.BLOB then
				p(" BLOB")
			end
			if t.PRIMARY then
				p(" PRIMARY KEY")
			elseif t.UNIQUE then
				p(" UNIQUE")
			end
			if t.DEFAULT then
				p(" DEFAULT ",t.DEFAULT) --- Only numbers? ...dont want defaults anyhow...
			end
		end
	end
	
--check if is already added
	local function in_table(tab,name)
		for i,v in ipairs(tab) do
			if v.name==name then return true end
		end
	end
	
	if not old then -- create new
	
		p("CREATE TABLE "..kind.."( ")
		for i,v in ipairs(info) do
			if i>1 then p(" , ") end
			pdef(v)
		end
		p(" );")
	
	else -- adjust
	
		local ch -- if set then we need to add these columns
		for i,v in ipairs(info) do
			if not in_table(old,v.name) then
				ch=ch or {}
				ch[#ch+1]=v
			end
		end

		if ch then
			for i,v in ipairs(ch) do
				p("ALTER TABLE "..kind.." ADD COLUMN ")
				pdef(v)
				p(" ;")
			end
		end
	end
	
	if t[1] then -- something to do
--		print(table.concat(t))
		exec(db,table.concat(t))
	end
	
end



-----------------------------------------------------------------------------
--
-- escape a string for sqlite use
--
-----------------------------------------------------------------------------
function escape(s)
	return "X'"..string.gsub(s, ".", function (c)
		return string.format("%02x", string.byte(c))
	end).."'"
--	return "'"..s:gsub("'","''").."'"
end


-----------------------------------------------------------------------------
--
-- turn a table into a string of values
--
-----------------------------------------------------------------------------
function make_values(tab)

	local ns={}
	local ds={}
	for n,d in pairs(tab) do
		ns[#ns+1]=fixname(n)
		ds[#ds+1]=fixvalue(d)
	end

	return "("..table.concat(ns,",")..")".." VALUES ("..table.concat(ds,",")..")"
end

-----------------------------------------------------------------------------
--
-- turn a table into a string of sets
--
-----------------------------------------------------------------------------
function make_valueset(tab)

	local ss={}
	for n,d in pairs(tab) do
		ss[#ss+1]=fixname(n)"="..fixvalue(d)
	end

	return table.concat(ss,",")
end

function fixvalue(v)
	if type(v)=="string" then
		return escape(v)
	else
		return tonumber(v)
	end
end

function fixname(v)
	return '"'..(tostring(v))..'"' -- name must not include " or '
end

-----------------------------------------------------------------------------
--
-- insert or update data on clash, similar format to the lanes returned info
-- this function doesnt do anything it just builds a queery string that will
--
-- name == table name
-- tab == data to insert
--
-- this is a single insert or update, so there is only one row
--
-- any previous values not in the tab be lost, use update to only change some values
--
-----------------------------------------------------------------------------
function make_replace(name,tab)
	return "REPLACE INTO "..name.." "..make_values(tab)..";"
end

-----------------------------------------------------------------------------
--
-- insert only, similar format to the lanes returned info
-- this function doesnt do anything it just builds a queery string that will
--
-- name == table name
-- tab == data to insert
--
-- this is a single insert or update, so there is only one row
--
-----------------------------------------------------------------------------
function make_insert(name,tab)
	return "INSERT INTO "..name.." "..make_values(tab)..";"
end

-----------------------------------------------------------------------------
--
-- update only, similar format to the lanes returned info
-- this function doesnt do anything it just builds a queery string that will
--
-- name == table name
-- tab == data to insert
-- where == where to update (sql string)
--
-- this is a single insert or update, so there is only one row
--
-----------------------------------------------------------------------------
function make_update(name,tab,where)

	return "UPDATE "..name.." SET "..make_valueset(tab).." WHERE "..where..";"
end

-----------------------------------------------------------------------------
--
-- convert a table containing data about a query into an sqlite query string
--
-- this table was originally based around the restrictions of googles big table
-- so yes, it is a very limited subset
--
-----------------------------------------------------------------------------
function make_query(tab)

	local operators={ -- require the use of C style ops
		["=="]="=",
		["<"]="<",
		[">"]=">",
		["<="]="<=",
		[">="]=">=",
		["!="]="<>",
		}

	local t={}
	local p=function(...)
		for i,v in ipairs{...} do t[#t+1]=tostring(v) end
	end
	
	p("SELECT *,ROWID FROM ",tab.kind," ")
	
	local wa="WHERE"
	for i,v in ipairs(tab) do
		
		if v[1]=="filter" then
		
			local o=operators[ v[3] ] 
			if o then
				p(wa," ",fixname(v[2]),o,fixvalue(v[4])," ")
			elseif string.upper(v[3])=="IN" then
				p(wa," ",fixname(v[2]),"IN(")
				for i,v in ipairs(v[4]) do
					if i~=1 then p(",") end -- separator
					p(fixvalue(v))
				end
				p(") ")
			else
				error("UNSUPORTED SQLITE OPERATOR "..tostring(v[3]))
			end
		
			wa="AND" -- switch from WHERE to AND
		end
	end

	local ss={}
	for i,v in ipairs(tab) do
		if v[1]=="sort" then
			if v[3]=="DESC" then
				ss[#ss+1]=fixname(v[2]).." DESC"
			
			else
				ss[#ss+1]=fixname(v[2]).." ASC"
			end		
		end
	end
	if ss[1] then
		p("ORDER BY ")
		p(table.concat(ss," , "))
		p(" ")
	end
	
	if tab.limit then
		p("LIMIT ",tab.limit," ")
	end

	if tab.offset then
		p("OFFSET ",tab.offset," ")
	end
	
	p(";")

	local s=table.concat(t)
	return s
end



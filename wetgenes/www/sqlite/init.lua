-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=print

if ngx then
	log=require("wetgenes.www.any.log").log
end

local sql=require("sqlite")
local wstr=require("wetgenes.string")

module(...)

dbs={} -- tooglobal?pass in this or your own to the funcs anyhow

function open(dbs,prefix,name,postfix) -- multiple opens are ok and get you the same db

	local db=dbs[name]
	
	if db then return db end -- already open
	
	db=assert(sql.open(prefix..name..postfix))

	set_pragmas(db) -- always run this

	return db
	
end

function close(dbs,name)

	local db=dbs[name]
	
	if db then
		sb:close()
		dbs[name]=nil
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

	if 	db:exec(s,f,d)~=sql.OK then error(db:errmsg()) end

end

-- get all rows the sql requests
function rows(db,s)

	local d={}
	local f=function(d,count,v,n)
		for i=1,count do
			dd={}
			dd[ n[i] ]=v[i]
			d[#d+1]=dd
		end
		return 0
	end

	if 	db:exec(s,f,d)~=sql.OK then error(db:errmsg()) end

	return d
end

-- get first row the sql requests
function row(db,s)
	return rows(db,s)[1]
end

-- get info about a table, this can only work if WE created the table
function get_info(db,name)

--[[
	local d=rows(db,"PRAGMA table_info('"..name.."')");
	print(wstr.serialize(d))
]]

	local d=rows(db,"select sql from sqlite_master where name = '"..name.."';")
	
	if not d[1] then return end -- no table of the given name exists
	
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
function set_info(db,name,info)


--	print(wstr.serialize(info))

	old=get_info(db,name)

-- build the sql string we need to run	
	local t={}
	local p=function(...) for i,v in ipairs{...} do t[#t+1]=tostring(v) end end

-- add a column
	local function pdef(t)
		if t.name then
			p(t.name)
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
	
		p("CREATE TABLE "..name.."( ")
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
				p("ALTER TABLE "..name.." ADD COLUMN ")
				pdef(v)
				p(" ;\n")
			end
		end
	end
	
	print(table.concat(t))
	exec(db,table.concat(t))
	
end

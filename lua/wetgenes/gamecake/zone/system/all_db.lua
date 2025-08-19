--
-- (C) 2025 Kriss@XIXs.com
--

local cmsgpack=require("cmsgpack")

local djon=require("djon")
local wjson=require("wetgenes.json")
local fats=require("wetgenes.fats")

local get_sql_to=require("wetgenes.getsql").sqlite

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local all=require("wetgenes.gamecake.zone.system.all") -- put everything in here

-- database functions
all.db=all.db or {}
M.db=all.db

-- all.db contains all the methods
all.db.meta={__index=all.db}

all.db.table_columns={
	{ name="uid",    NOT_NULL=true ,  INTEGER=true ,  PRIMARY=true }, -- fully unique
	{ name="time",   REAL=true     ,  INDEX=true   }, -- last updated timestamp for this object
	{ name="boot",   TEXT=true     ,  JSON=true    }, -- full boot data *not* including the zip blob or uid
	{ name="zip",    BLOB=true     },
}
all.db.table_columns_type=get_sql_to.columns_types(all.db.table_columns)

-- convert a json encoded sql row to an unzipped boot
all.db.row_to_boot=function(db,row)
	local boot=djon.load(row.boot)
	boot.uid=row.uid
	boot.zip=row.zip
	return boot
end

-- convert a boot to a json encoded sql row
all.db.boot_to_row=function(db,boot)
	local row={}
	row.uid=boot.uid
	boot.uid=nil
	if not next(boot) then return nil end -- no data
	row.zip=boot.zip
	boot.zip=nil
	row.boot=djon.save(boot,"compact")
	row.time=db.time()
	return row
end


-- create and return db
all.db.open=function(opts)
	local db={}
	setmetatable(db,all.db.meta)
	
	db.linda=assert(opts.linda) -- passing in a linda is required
	db.time=assert(opts.time) -- passing in a time function is required
	db.table_name=opts.table_name or "zones" -- can choose table name for data

	db:do_memo({
		task="sqlite",
		sql=get_sql_to.create_table( db.table_name , db.table_columns ),
	})

	for i,sql in ipairs( get_sql_to.create_table_indexs( db.table_name , db.table_columns ) ) do
		db:do_memo({
			task="sqlite",
			sql=sql,
		})
	end
	
	-- need tp create some more custom json indexes here for all the boot data

	return db
end

-- auto linda from db
all.db.do_memo=function(db,...) return all.do_memo(db.linda,...) end
all.db.memos=function(db,...) return all.memos(db.linda,...) end


-- write this change of boot data to the database
all.db.save_boot=function(db,boot)

	local row=db:boot_to_row(boot)
	
	if not row then -- nothing to update
		return
	end

	-- probably not updating zip	
	local sql=[[
INSERT INTO ]]..db.table_name..[[ ( uid , time , boot ) VALUES ( $uid , $time , $boot )
ON CONFLICT( uid ) DO UPDATE SET uid=$uid , time=$time , boot=json_patch( boot , $boot ) ;
]]
	-- but if we are then
	if row.zip then
		sql=[[
INSERT INTO ]]..db.table_name..[[ ( uid , time , boot , zip ) VALUES ( $uid , $time , $boot , $zip )
ON CONFLICT( uid ) DO UPDATE SET uid=$uid , time=$time , boot=json_patch( boot , $boot ) , zip=$zip ;
]]
	end

	-- dont ask for a response, assume it all went well
	db:do_memo({
		task="sqlite",
		id=false,
		sql=sql,
		binds={ uid=row.uid , time=row.time , boot=row.boot , zip=row.zip and true },
		blobs={ zip=row.zip },
	})
	
	-- print error but keep going
--	if ret.error then print(sql,ret.error) end

end


--[[
	Get boot that matches this partial boot, normally by uid eg 
	{uid=1234} but any root parts of boot can be explicitly matched and 
	multiple boots may be returned
--]]
all.db.get_boots=function(db,boot,recursive)
	local qs={}
	local vs={}
	
	for n,v in pairs(boot) do
		qs[#qs+1]=qs[1] and " AND " or nil -- only between
		if n=="uid" then -- explicit uid is not in boot
			qs[#qs+1]=" "..n.."=$"..n.." "
		else
			qs[#qs+1]=" json_extract(boot, '$."..n.."' )=$"..n.." "
		end
		vs[n]=v
	end
	
	local sql=[[
SELECT uid,time,boot,zip FROM ]]..db.table_name..[[ t1 WHERE ]]..table.concat(qs)..[[
]]

	if recursive then -- recurse all dependencies using boot.uids arrays
	
		sql=[[
WITH RECURSIVE list(uid,time,boot,zip) AS
( ]]..sql..[[
UNION ALL
SELECT t1.uid,t1.time,t1.boot,t1.zip FROM list JOIN ]]..db.table_name..[[ t1
WHERE t1.uid IN ( SELECT value FROM json_each(list.boot, '$.uids') )
)
SELECT uid,time,boot,zip FROM list
]]

	end

	-- dont ask for a response, assume it all went well
	local ret=db:do_memo({
		task="sqlite",
		sql=sql..";",
		binds=vs,
	})
	if ret.error then return nil,ret.error end
	
	local boots={}
	for i,row in ipairs(ret.rows) do
		boots[#boots+1]=db:row_to_boot(row)
	end
	return boots
end

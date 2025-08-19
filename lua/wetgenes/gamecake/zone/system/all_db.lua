--
-- (C) 2025 Kriss@XIXs.com
--

local cmsgpack=require("cmsgpack")

local djon=require("djon")
local wjson=require("wetgenes.json")
local fats=require("wetgenes.fats")

local get_sql_to=require("wetgenes.getsql").sqlite

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
	{ name="boot",   TEXT=true     ,  JSON=true    }, -- full boot data *not* including the zip blob
	{ name="zip",    BLOB=true     },
}
all.db.table_columns_type=get_sql_to.columns_types(all.db.table_columns)

-- convert a json encoded sql row to an unzipped boot
all.db.row_to_boot=function(row)
	local boot=djon.load(row.boot)
	boot.uid=row.uid
	boot.zip=row.zip
	return boot
end

-- convert a boot to a json encoded sql row
all.db.boot_to_row=function(boot)
	local row={}
	row.uid=boot.uid
	boot.uid=nil
	row.zip=boot.zip
	boot.zip=nil
	row.boot=djon.save(boot,"compact")
	row.time=0
	return row
end


-- create and return db
all.db.open=function(opts)
	local db={}
	setmetatable(db,all.db.meta)
	
	db.linda=assert(opts.linda) -- passing in a linda is required
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

	local row=db.boot_to_row(boot)

	-- probably not updating zip	
	local sql=[[
INSERT INTO	]]..db.table_name..[[ ( uid , time , boot ) VALUES ( $uid , $time , $boot )
ON CONFLICT( uid ) DO UPDATE SET uid=$uid , time=$time , boot=json_patch( boot , $boot ) ;
]]
	-- but if we are then
	if row.zip then
		sql=[[
INSERT INTO	]]..db.table_name..[[ ( uid , time , boot , zip ) VALUES ( $uid , $time , $boot , $zip )
ON CONFLICT( uid ) DO UPDATE SET uid=$uid , time=$time , boot=json_patch( boot , $boot ) , zip=$zip ;
]]
	end

	local ret=db:do_memo({
		task="sqlite",
		sql=sql,
		binds={ uid=row.uid , time=row.time , boot=row.boot , zip=row.zip and true },
		blobs={ zip=row.zip },
	})
	
	-- print error but keep going
	if ret.error then print(sql,ret.error) end

end

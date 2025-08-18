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
	{ name="uid",    NOT_NULL=true ,  INTEGER=true ,  PRIMARY=true },
	{ name="stage",  NOT_NULL=true ,  INTEGER=true ,  INDEX=true   },
	{ name="caste",  NOT_NULL=true ,  TEXT=true    ,  INDEX=true   },
	{ name="idz",    INTEGER=true  ,  INDEX=true   },
	{ name="pidx",   INTEGER=true  ,  INDEX=true   }, -- index into uids of parent link ( and a flag that we are a childof )
	{ name="id",     TEXT=true     ,  INDEX=true   },
	{ name="uids",   TEXT=true     ,  JSON=true    },
	{ name="zip",    BLOB=true     },
	{ name="time",   REAL=true     ,  INDEX=true   }, -- last updated time of this object in seconds
	{ name="siz",    TEXT=true     ,  JSON=true    }, -- json will mostly be arrays of 3 or 4 numbers
	{ name="pos",    TEXT=true     ,  JSON=true    },
	{ name="rot",    TEXT=true     ,  JSON=true    },
	{ name="vel",    TEXT=true     ,  JSON=true    },
	{ name="rez",    TEXT=true     ,  JSON=true    },
	{ name="ang",    TEXT=true     ,  JSON=true    },
	{ name="dot",    TEXT=true     ,  JSON=true    },
}
all.db.table_columns_type=get_sql_to.columns_types(all.db.table_columns)

-- convert a json encoded sql row to an unzipped boot
all.db.row_to_boot=function(row)
	local boot={}
	for n,v in pairs( row ) do
		if "JSON"==(all.db.table_columns_type[n] or "JSON") then
			boot[n]=wjson.decode(v)
		else
			boot[n]=v
		end
	end
	if type(boot.zip)=="string" then -- need to unzip
		boot.zip=all.decode(boot.zip)
	end

	return boot
end

-- convert a boot to a json encoded sql row
all.db.boot_to_row=function(boot)
	local row={}
	for n,v in pairs( boot ) do
		if "JSON"==(all.db.table_columns_type[n] or "JSON") then
			row[n]=wjson.encode(v)
		else
			row[n]=v
		end
	end
	if type(row.zip)=="table" then -- need to zip
		row.zip=all.encode(row.zip)
	end

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

	return db
end

-- auto linda from db
all.db.do_memo=function(db,...) return all.do_memo(db.linda,...) end
all.db.memos=function(db,...) return all.memos(db.linda,...) end


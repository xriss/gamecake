--
-- (C) 2025 Kriss@XIXs.com
--

-- module
local M={ modname = (...) } package.loaded[M.modname] = M


-- unix paths and we use // for internal data so //data/ is not expected to hit the filesystem


------------------------------------------------------------------------
do -- only cache this stuff on main thread
------------------------------------------------------------------------
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

M.functions={}
M.metatable={__index=M.functions}
setmetatable(M,M.metatable)

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local djon=require("djon")
local wpath=require("wetgenes.path")
local wgetsql=require("wetgenes.getsql")
local wzips=require("wetgenes.zips")

local _,lfs=pcall( function() return require("lfs") end ) ; lfs=_ and lfs

local zlib=require("zlib")
local zip_inflate=function(d) return ((zlib.inflate())(d)) end
local zip_deflate=function(d) return ((zlib.deflate())(d,"finish")) end

local wgw_mounts=require("wetgenes.gamecake.widgets.mounts")

local function dprint(a) print(wstr.dump(a)) end

M.default_configs={}
do
	local default_config_filename="lua/swanky/edit/config.djon"
	local default_config=assert(wzips.readfile(default_config_filename),"file not found: "..default_config_filename)
	local t=djon.load(default_config,"comment")
	for n,v in pairs(t[1]) do -- comment format so tables within tables
		M.default_configs[n]=djon.save(v,"djon","comment") -- reformat keeping comments
	end
end

M.tables=wgetsql.prepare_tables({

-- djon data with comments so intended to be human editable
-- this means we must keep comments on read/write cycles
-- top level values are considered keys in the config object
	config={
		{	"key",		"TEXT",		"PRIMARY",	},
		{	"value",	"TEXT",					},
	},

-- single row per file
	file={
		{	"id",		"INTEGER",	"PRIMARY",	},
		{	"path",		"TEXT",		"UNIQUE",	},
		{	"dir",		"TEXT",		"INDEX",	},
		{	"name",		"TEXT",		"INDEX",	},
		{	"meta",		"JSONB",				},
	},

-- single row per file ( cache of last save/load file contents )
	file_data={
		{	"id",		"INTEGER",	"PRIMARY",	},
		{	"ud",		"INTEGER",				},
		{	"value",	"BLOB",					},
	},

-- multiple rows per file so id can not be primary and each row is an undo change
	file_undo={
		{	"id",		"INTEGER",	"INDEX",	},
		{	"ud",		"INTEGER",				},
		{	"value",	"BLOB",					},
		{	UNIQUE={"id","ud"},					},
	},

})

-- collection of state backed up to an sqlite database continuously

M.bake=function(oven,collect)

	collect.oven=oven -- back link for meta access

	collect.mounts_config=wgw_mounts.config.setup({
		name="config/",
		path="//config/",
		dir={},
		collect=collect, -- back link to this state data
	})
	collect.mounts_gist=wgw_mounts.gist.setup({
		name="gists/",
		path="//gists/",
		dir={},
		collect=collect, -- back link to this state data
	})
	collect.mounts_meta=wgw_mounts.meta.setup({
		name="//",
		path="//",
		dir={},
		dir_auto={ collect.mounts_config , collect.mounts_gist },
		keep=true,
	})
	collect.mounts_file=wgw_mounts.file.setup({
		name="/",
		path="/",
		dir={},
		keep=true,
	})
	collect.mounts=wgw_mounts.meta.setup({
		name="",
		path="",
		dir={ collect.mounts_meta , collect.mounts_file },
		keep=true,
	})

--[[
	collect.config_mounts=function(config)
		if config.gists then
			local parent=collect.mounts_gist
			parent.dir={} -- reset
			if config.gists.users then
				for _,name in ipairs( config.gists.users ) do
					local it=wgw_mounts.gist.setup({
						path=parent.path..name.."/",
						name=name.."/",
						gist_user=name,
						dir={},
					})
					it.keep=true
					it.parent=parent
					parent.dir[#parent.dir+1]=it
				end
			end
			if config.gists.gists then
				local it=wgw_mounts.gist.setup({
					path=parent.path..".../",
					name=".../",
					gist_user=".../",
					dir={},
				})
				it.keep=true
				it.parent=parent
				parent.dir[#parent.dir+1]=it
				do
					local parent=it
					for _,id in ipairs( config.gists.gists ) do
						local it=wgw_mounts.gist.setup({
							path=parent.path..id.."/",
							name=id.."/",
							gist_id=id,
							dir={},
						})
						it.keep=true
						it.parent=parent
						parent.dir[#parent.dir+1]=it
					end
				end
			end
		end
	end
]]
	
	collect.task_id=collect.task_id or "collect"
	collect.task_id_msg=collect.task_id..":msg"

	collect.send_memo=function(it)
		it.task=collect.task_id
		oven.tasks:send(it,0)
	end

	collect.do_memo=function(it,noerror)
		it.task=collect.task_id
		local ret=oven.tasks:do_memo(it)
		if ret.error and ( not noerror ) then error(ret.error) end -- auto raise any SQL errors
		return ret
	end
	
	local load_meta=function(path)
		local it=collect.do_memo({
			binds={
				PATH=path,
			},
			sql=[[

	SELECT id,path,json(meta) AS meta FROM file WHERE path=$PATH;

			]],
		}).rows[1]
		if it then -- maybe null
			it.meta=djon.load(it.meta)
			it.meta.id=it.id
			it.meta.path=it.path
			return it.meta
		end
	end

	local manifest_meta=function(path)
		local it=load_meta(path)
		if it then return it end

		local meta={}
		meta.undo=0	-- a new file will have an undo level of 0
		meta.state="manifest" -- this is a new file

		-- write meta
		collect.do_memo({
			binds={
				PATH=path,
				DIR=wpath.unslash( wpath.dir(path) ),
				NAME=wpath.file(path),
				META=djon.save( meta ,"compact" ),
			},
			sql=[[

	INSERT INTO file (path,dir,name,meta)
	VALUES ( $PATH,$DIR,$NAME,jsonb($META) )

			]],
		})

		-- read meta so we know ID
		return load_meta(path)
	end

	local save_meta=function(meta)

		-- write meta to sqlite
		collect.do_memo({
			binds={
				ID=meta.id,
				META=djon.save( meta ,"compact" ),
			},
			sql=[[

	UPDATE file SET meta=jsonb($META) WHERE id=$ID;

			]],
		})

	end

	collect.setup=function()

		-- create collect data handling thread
		collect.thread=oven.tasks:add_global_thread({
			count=1,
			id=collect.task_id,
			globals={
				sqlite_filename=wwin.files_prefix.."swed.collect.sqlite",
				sqlite_pragmas=[[ ; ]],
				sqlite_tables=M.tables,
				default_configs=M.default_configs,
			},
			code=M.task_code,
		})
				
		-- global json state data contained in keys, read on startup and should be written when changed
		collect.config={}

		-- block and read all the json
		local djon_config=collect.do_memo({
			sql=[[

SELECT key,value FROM config ;

			]],
		})
		for i,v in ipairs(djon_config.rows) do
			collect.config[v.key]=djon.load(v.value) -- this loses comments
		end
		
--		collect.config_mounts( collect.config )
		
--		DUMP( collect.config )

	end

-- load the file from database first then disk and also cache this file in database
	collect.load=function(it,path)
	
		it.meta=manifest_meta(path)

		if it.meta.state=="manifest" then -- not a real meta yet so load from disk and update

			local text=collect.mounts:read_file(path) or ""
		
			-- write data only if we have some
			collect.do_memo({
				binds={
					ID=it.meta.id,
					UD=0,
				},
				blobs={
					DATA=zip_deflate(text),
				},
				sql=[[

	INSERT INTO file_data (id,ud,value)
	VALUES ( $ID,$UD,$DATA )

				]],
			})
			
			it.meta.state="new"
			save_meta(it.meta)

		end

-- always load from sqlite even if we just saved to it

		local data=collect.do_memo({
			binds={
				ID=it.meta.id,
			},
			sql=[[

	SELECT id,ud,value FROM file_data WHERE id=$ID;

			]],
		}).rows[1]
		if data then
			it.meta.undo=data.ud -- the undo point that this data was saved at
			it.txt.set_text( zip_inflate(data.value) ,filename) -- set the uncompressed text

			data=nil -- dont bother keeping the compressed data around
		end

		local undos=collect.do_memo({
			binds={
				ID=it.meta.id,
			},
			sql=[[

	SELECT id,ud,value FROM file_undo WHERE id=$ID;

			]],
		}).rows or {}
		it.txt.undo.list_set_fromsql(undos,it.meta.undo)
		it.txt.undo.redo_all()

	end

-- save an undo update ( possibly overwrite )
	collect.undo_update=function(it,index,data)
		if not it.meta then return end -- need meta id

		collect.send_memo({
			binds={
				ID=it.meta.id,
				UD=index,
			},
			blobs={
				DATA=data,
			},
			sql=[[

	INSERT INTO file_undo ( id, ud, value )
	VALUES ( $ID, $UD, $DATA )
	ON CONFLICT DO UPDATE SET
	id=$ID,	ud=$UD,	value=$DATA ;

			]],
		})
	end

-- trim undos to the given index
	collect.undo_trim=function(it,index)
		if not it.meta then return end -- need meta id

		collect.do_memo({
			binds={
				ID=it.meta.id,
				UD=index,
			},
			sql=[[

	DELETE FROM file_undo WHERE id=$ID AND ud>$UD;

			]],
		})
	end

-- save the file from database first then disk
	collect.save=function(it,path)

		if path and path~=it.meta.path then -- saving to new file
			it.meta=manifest_meta(path) -- this will generate a new meta ( or load it from disk)
			-- need to reset any undos this file may already have
		end
		
		local text=it.txt.get_text()
		
		-- write data to disk
		local retext=collect.mounts:write_file( it.meta.path , text )
		if retext and retext~=text then -- doc was auto updated on save
			it.txt.mark(0,0,it.txt.hy+1,0)
			it.txt.undo.replace(retext)
			text=retext
		end

		it.meta.undo=it.txt.undo.index
		it.meta.state="save"
		save_meta(it.meta)

		-- write data to sqlite
		collect.do_memo({
			binds={
				ID=it.meta.id,
				UD=it.meta.undo,
			},
			blobs={
				DATA=zip_deflate(text),
			},
			sql=[[

	UPDATE file_data SET ud=$UD,value=$DATA WHERE id=$ID;

			]],
		})


	end
	
	return collect
end

------------------------------------------------------------------------
end -- The functions below are free running tasks and should not depend on any locals
------------------------------------------------------------------------


--[[#lua.swanky.edit.collect.task_code

lanes task function for handling collect communication.

Mostly we are read/writing an sqlite database containing constantly updated state info.

]]
M.task_code=function(linda,task_id,task_idx)
	local M -- hide M for thread safety
	local global=require("global") -- lock accidental globals
	local lanes=require("lanes")
	local task_id_collect=task_id..":collect"
	if lane_threadname then lane_threadname(task_id) end

	local wgetsql=require("wetgenes.getsql")
	
	local sqlite3 = lanes.require("lsqlite3")
	local db
	
	local opendb=function(filename,pragmas,tables)
		if db then db:close() end
		db=nil
		if not filename then return end -- close 

		db = assert(sqlite3.open(filename))

		if pragmas then
			db:exec(pragmas)
		end
		
		if tables then
			for tabname,tab in pairs(tables) do
				local sql=wgetsql.sqlite.create_table(tab.name,tab)
				db:exec(sql)
				for _,sql in ipairs( wgetsql.sqlite.alter_table(tab.name,tab) ) do
					db:exec(sql)
				end
				for _,sql in ipairs( wgetsql.sqlite.create_table_indexs(tab.name,tab) ) do
					db:exec(sql)
				end
			end
		end
		
		if default_configs then -- auto create
		
			local keys={}
			for key in db:urows([[
				SELECT key FROM config ;
			]]) do
				keys[key]=true
			end
			
			for key,value in pairs(default_configs) do
				if not keys[key] then
					local stmt = db:prepare[[ INSERT INTO config VALUES (:key, :value) ]]
					stmt:bind_names{  key = key,  value = value    }
					stmt:step()
					stmt:finalize()
				end
			end

		end
		
		return db
	end
	opendb( sqlite_filename , sqlite_pragmas , sqlite_tables , default_configs ) -- auto open and pragma and create tables

	local request=function(memo)
	
		local ret={}
	
		if memo.cmd then -- this is a special cmd eg to close or open the database
		
			if memo.cmd=="open" then -- gonna need to do this first or set sqlite_filename to auto open

				opendb( memo.filename , memo.pragmas , memo.tables )

			elseif memo.cmd=="close" then -- probably good to "try" and do this before exiting

				opendb() -- yeah uhm so no filename means we are just going to close the current db

			end

		elseif memo.sql then -- execute some sql
		
			if not db then
				ret.error="no database"
				return ret
			end

			local rows={}
			
			
			local err
			
			if memo.binds or memo.blobs then -- use prepared statement
			
				local stmt = db:prepare(memo.sql)
				if not stmt then
					ret.error=db:errmsg()
					return ret
				end

				local bmax=stmt:bind_parameter_count()
				local bs={}
				for i=1,bmax do
					local n=stmt:bind_parameter_name(i)
					if n then
						bs[n]=i
						bs[n:sub(2)]=i
					end
				end

				
				local blobs=memo.blobs or {}
				for n,v in pairs( memo.binds or {} ) do
					if bs[n] and not blobs[n] then -- a blob might be in both places
						stmt:bind( bs[n] , v )
					end
				end
				for n,v in pairs( memo.blobs or {} ) do -- these binds should be treated as blobs
					if bs[n] then
						stmt:bind_blob( bs[n] , v )
					end
				end
				
				if memo.compact then
					rows.names=stmt:get_names()
					for it in stmt:rows() do
						rows[#rows+1]=it
					end
				else
					for it in stmt:nrows() do
						rows[#rows+1]=it
					end
				end

				err=stmt:finalize()
			
			else
			
				if memo.compact then -- return data in a slightly more compact format

					err=db:exec(memo.sql,function(udata,cols,values,names)
						rows.names=names
						rows[#rows+1]=values
						return 0
					end,"udata")
				
				else

					err=db:exec(memo.sql,function(udata,cols,values,names)
						local it={}
						for i=1,cols do it[ names[i] ] = values[i] end
						rows[#rows+1]=it
						return 0
					end,"udata")

				end

			end

			if err~=sqlite3.OK then
				ret.error=db:errmsg()
			else
				ret.rows=rows
			end

		end
		
		return ret
	end


	while true do

		local _,memo= linda:receive( 0 , task_id ) -- wait for any memos coming into this thread

		if memo then
			local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end

end

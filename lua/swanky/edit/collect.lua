--
-- (C) 2025 Kriss@XIXs.com
--

-- module
local M={ modname = (...) } package.loaded[M.modname] = M


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
local wpath=require("wetgenes.path")
local wgetsql=require("wetgenes.getsql")

local _,lfs=pcall( function() return require("lfs") end ) ; lfs=_ and lfs

local function dprint(a) print(wstr.dump(a)) end


M.tables=wgetsql.prepare_tables({

	data={
		{	"key",		"TEXT",		"PRIMARY",	},
		{	"value",	"JSON",					},
	},

	file={
		{	"path",		"TEXT",		"PRIMARY",	},
		{	"dir",		"TEXT",					},
		{	"name",		"TEXT",					},
	},

})

dprint(M.tables)

-- collection of state backed up to an sqlite database continuously

M.bake=function(oven,collect)

	collect.task_id=collect.task_id or "collect"
	collect.task_id_msg=collect.task_id..":msg"

	collect.setup=function()

		-- create collect data handling thread
		collect.thread=oven.tasks:add_global_thread({
			count=1,
			id=collect.task_id,
			globals={
				sqlite_filename=wwin.files_prefix.."swed.collect.sqlite",
				sqlite_pragmas=[[ ; ]],
				sqlite_tables=M.tables,
			},
			code=M.task_code,
		})
		
		-- global json state data contained in keys, read on startup and should be written when changed
		collect.data={}

		-- block and read all the json
		local json_data=oven.tasks:do_memo({
			task=collect.task_id,
			sql=[[

SELECT * FROM data ;

			]],
		})
print(json_data.error)
print(json_data.rows)
		for i,v in ipairs(json_data.rows) do
			collect.data[v.key]=v.value
		end

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
			end
		end
		
		return db
	end
	opendb( sqlite_filename , sqlite_pragmas , sqlite_tables ) -- auto open and pragma and create tables

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


--[[


Get SQL code for various databases from slightly structured inputs

This is hacky and in progress but provides some abstraction for 
switching.

]]


local getsql={ modname=(...) } ; package.loaded[getsql.modname]=getsql

getsql.sqlite_meta={ __index=getsql }
getsql.sqlite={}
setmetatable(getsql.sqlite, getsql.sqlite_meta)


-- convert simple lists into table data
getsql.prepare_tables=function(tables)

	for tabname,tab in pairs(tables) do
		tab.name=tabname
		tab.columns={} -- keyed by column name
		for idx,col in ipairs(tab) do
			col.index=idx
			for i,val in ipairs(col) do
				if i==1 then -- first is name
					col.name=val
					tab.columns[col.name]=col
				else		-- everything else is auto flags
					col[val]=true
				end
			end
		end
	end
	
	return tables
end

getsql.sqlite.columns_types=function(columns,types)
	types=types or {}
	for n,v in pairs(columns) do
		local t="TEXT"
		if v.INTEGER then t="INTEGER" end
		if v.REAL    then t="REAL"    end
		if v.BLOB    then t="BLOB"    end
		if v.JSON    then t="JSON"    end
		types[v.name]=t
	end
	return types
end

getsql.sqlite.create_table=function(name,columns)

	local ss={}
	local push=function(s) ss[#ss+1]=s end
	
	push("CREATE TABLE IF NOT EXISTS "..name.." (\n")
	
	local column_count=0
	for _,column in ipairs(columns) do

		if column.name then

			column_count=column_count+1
			if column_count>1 then push(" ,\n") end
	
			push(" "..column.name.." ")
			
			if     column.INTEGER  then push(" INTEGER ")
			elseif column.REAL     then push(" REAL ")
			elseif column.BLOB     then push(" BLOB ")
			elseif column.TEXT     then push(" TEXT ")
			elseif column.NOCASE   then push(" TEXT ")
			elseif column.JSON     then push(" TEXT ")
			end

			if     column.NOT_NULL then push(" NOT NULL ")
			end
			
			if     column.PRIMARY  then push(" PRIMARY KEY ")
			elseif column.UNIQUE   and  ( type(column.UNIQUE) ~= "table" )  then push(" UNIQUE ")
			end
			
			if     column.NOCASE   then push(" COLLATE NOCASE ")
			end

			if     column.DEFAULT   then push(" DEFAULT "..column.DEFAULT.." ")
			end

		end

	end

	for _,column in ipairs(columns) do

		if type(column.UNIQUE)=="table" then -- add unique constraints
			push(" , UNIQUE ("..table.concat(column.UNIQUE,",")..") \n")
		end

	end

	push(" );\n\n")

	return table.concat(ss,"");
end

-- returns array of strings that should be run and ignored if they fail
getsql.sqlite.alter_table=function(name,columns)

	local rets={}
	local ss={}
	local push=function(s) ss[#ss+1]=s end
	
	for _,column in ipairs(columns) do
		ss={}
		if column.name then

			push("ALTER TABLE "..name.." ADD COLUMN ")
			
			push(" "..column.name.." ")
			
			if     column.INTEGER  then push(" INTEGER ")
			elseif column.REAL     then push(" REAL ")
			elseif column.BLOB     then push(" BLOB ")
			elseif column.TEXT     then push(" TEXT ")
			elseif column.NOCASE   then push(" TEXT ")
			elseif column.JSON     then push(" TEXT ")
			end

			if     column.NOT_NULL then push(" NOT NULL ")
			end

			if     column.NOCASE   then push(" COLLATE NOCASE ")
			end

			rets[#rets+1]=table.concat(ss,"").." ; "
		end

	end

	return rets
end


-- returns array of strings that should be run to create indexs
getsql.sqlite.create_table_indexs=function(name,columns)

	local rets={}
	local push=function(s) rets[#rets+1]=s end

	for _,column in ipairs(columns) do

		if column.INDEX then -- add index
			push("CREATE INDEX IF NOT EXISTS idx_"..name.."_"..column.name.." ON "..name.."( "..column.name.." );\n\n")
		end

		if type(column.UNIQUE)=="table" then
			push("CREATE UNIQUE INDEX IF NOT EXISTS idx_"..table.concat(column.UNIQUE,"_").." ON "..name.." ( "..table.concat(column.UNIQUE," , ").." );\n\n")
		elseif column.UNIQUE then -- add unique index
			push("CREATE UNIQUE INDEX IF NOT EXISTS idx_"..column.name.." ON "..name.."( "..column.name.." );\n\n")
		end

	end

	return rets
end

getsql.sqlite.upsert=function(name,columns,key)
	local ss={}
	local push=function(s) ss[#ss+1]=s end
	
	local order={}
	local idx=0
	for n,v in pairs(columns) do
		idx=idx+1
		order[idx]=n
	end
	
	push("INSERT INTO "..name.." ( ")
	push(table.concat(order,", "))
	push(" )\n")

	push(" VALUES ( ")
	push("$"..table.concat(order,", $"))
	push(" )\n")
	
	push(" ON CONFLICT("..key..") DO UPDATE SET\n")
	local idx=0
	for i,n in ipairs( order ) do
		if n~=key then
			idx=idx+1
			if idx>1 then push(" , ") end
			push("  "..n.."=$"..n.." ")
		end
	end
	push(";\n\n")
--	push("\n WHERE "..key.."=$"..key.." ;\n")

	return table.concat(ss,"");
end


getsql.sqlite.insert=function(name,columns,key)
	local ss={}
	local push=function(s) ss[#ss+1]=s end
	
	local order={}
	local idx=0
	for n,v in pairs(columns) do
		idx=idx+1
		order[idx]=n
	end
	
	push("INSERT INTO "..name.." ( ")
	push(table.concat(order,", "))
	push(" )\n")

	push(" VALUES ( ")
	push("$"..table.concat(order,", $"))
	push(" );\n")
	
	return table.concat(ss,"");
end

getsql.sqlite.update=function(name,columns,key)
	local ss={}
	local push=function(s) ss[#ss+1]=s end
	
	local order={}
	local idx=0
	for n,v in pairs(columns) do
		idx=idx+1
		order[idx]=n
	end
	
	push("UPDATE "..name.." SET\n")
	local idx=0
	for i,n in ipairs( order ) do
		if n~=key then
			idx=idx+1
			if idx>1 then push(" , ") end
			push("  "..n.."=$"..n.." ")
		end
	end
	push(" WHERE "..key.."=$"..key..";\n\n")
		
	return table.concat(ss,"");
end

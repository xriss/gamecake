#!/usr/local/bin/gamecake

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()



local sqlite3=require("lsqlite3")

local db = sqlite3.open_memory()

db:exec[[
  CREATE TABLE test (id INTEGER PRIMARY KEY, content JSON);

  INSERT INTO test VALUES (NULL, 'Hello World');

]]

local pp=db:prepare[[
  INSERT INTO test VALUES (NULL, ?);
]]

local insert=function(data)
	pp:bind_values(data)
	pp:step()
	pp:reset()
end

insert(32)
insert(101)
insert("A String")
insert("some\nline")
insert("some\0zero")

insert(0xffffffff)
insert(0xffffffffffffffff)


for row in db:nrows("SELECT * FROM test") do
  print( row.id, row.content, type(row.content) , string.format("%q", tostring(row.content)) )
end



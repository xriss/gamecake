--
-- (C) 2025 Kriss@XIXs.com
--

-- module
local M={ modname = (...) } package.loaded[M.modname] = M

--[[#lua.wetgenes.tasks_gist

	local gist=require("wetgenes.tasks_gist")

Access github gists using the github api via the http tasks.

]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local djon=require("djon")

M.functions={}
M.metatable={__index=M.functions}
setmetatable(M,M.metatable)


M.functions.list=function(opts)
	local tasks=opts.tasks -- must be tasks as returned from wetgenes.tasks.create()
		
	local q=""
	do
		local t={}
		if opts.since    then t[#t+1]="since="..    opts.since    end
		if opts.per_page then t[#t+1]="per_page=".. opts.per_page end
		if opts.page     then t[#t+1]="page="..     opts.page     end
		if #t>0 then
			q="?"..table.concat(t,"&")
		end
	end

	local headers={}
	headers["Accept"]="application/vnd.github+json"
	headers["X-GitHub-Api-Version"]="2022-11-28"
	if opts.token then
		headers["Authorization"]="Bearer "..opts.token
	end
	
	local memo={headers=headers,method="GET",url="https://api.github.com/gists"..q}
	local result=tasks:http(memo)
	local body=djon.load(result.body or "{}")

	return body
end





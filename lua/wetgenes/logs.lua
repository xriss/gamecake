--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wtsv=require("wetgenes.tsv")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local logs = M

logs.export=function(env,...)
	local tab={...} ; for i=1,#tab do tab[i]=env[ tab[i] ] end
	return unpack(tab)
end

-- setup from command line args

logs.setup=function(args)

	if args.logs then -- change allow and block lists
		logs.allow=nil
		logs.block=nil
		if type(args.logs)=="string" then
			local mode="allow"
			for word in string.gmatch(args.logs, "[%+%-]*[^%+%-]*") do
				local sign=word:sub(1,1)
				if sign=="+" then
					mode="allow"
					word=word:sub(2)
				elseif sign=="-" then
					mode="block"
					word=word:sub(2)
				end
				logs[mode]=logs[mode] or {}
				if word~="" then
					logs[mode][word]=true
				end
			end
		end
	end

end


logs.allow=nil -- put a table here to enable

logs.block={
	oven=true,	-- reduce oven spam by default
	thread=true,	-- reduce thread spam by default
	setup=true,	-- do not usually need to know setup orderthis except when debugging strangeness
}

local wwin
logs.log = function(mode,...)
	local args={}
	for i=1,select("#", ...) do
		args[i]=tostring( select(i, ...) )
	end
	if type(mode)~="string" or mode=="" or mode=="line" then -- special print location in file
		mode="line"
		if logs.allow and ( not logs.allow[ mode ] ) then return end
		if logs.block and (     logs.block[ mode ] ) then return end
		local line="here"
		 -- this can be null if we are a tail call so need fake fallback
		local info=debug.getinfo(2) or {currentline=0,name="",short_src=""}
		print( mode , info.currentline , info.name , info.short_src , unpack(args) )
		return
	end

--[[
	if not wwin then wwin=require("wetgenes.win") end
	if wwin.files_prefix then
		local t={mode,...}
		for i,v in ipairs(t) do t[i]=tostring(v) end

		local fp=io.open( wwin.files_prefix.."logs.tsv" , "a" )
		fp:write( table.concat(t,"\t") .. "\n" )
		fp:close()
	end
]]

	if logs.allow and ( not logs.allow[ mode ] ) then return end
	if logs.block and (     logs.block[ mode ] ) then return end

-- track down where the damn debug junk is from :)
--[[
	local info=debug.getinfo(2)
	print( mode , info.currentline , info.name or "." , info.short_src )
]]
	print( mode , unpack(args) )

end

-- print to the main display
logs.display = function(...)
	if oven and oven.console and oven.console.display then
		oven.console.display( ... )
	end
end


-- show full table contents

logs.dump = function(...)
	logs.log( "dump" , logs.tostring(...) )
end

local logstring

logs.tostring = function(...)
	local tab={...} ; for i=1,#tab do tab[i]=logstring( tab[i] ) end
	return unpack(tab)
end

logstring= function(o,opts)

	opts=opts or {}
	opts.done=opts.done or {} -- only do tables once

	opts.indent=opts.indent or ""

	local fout=opts.fout

	if not fout then -- self call with a new function to build and return a string
		local ret={}
		opts.fout=function(...)
			for i,v in ipairs({...}) do ret[#ret+1]=v end
		end
		logstring(o,opts)
		opts.fout("\n")
		return table.concat(ret)
	end

	if type(o) == "number" then

		return fout(o)

	elseif type(o) == "boolean" then

		if o then return fout("true") else return fout("false") end

	elseif type(o) == "string" then

		return fout(string.format("%q", o))

	elseif type(o) == "table" then

		if opts.done[o] and opts.no_duplicates then
			fout(opts.indent,"\n",opts.indent,"{--[[DUPLICATE]]}\n")
			return
		else

			fout("{\n")

			opts.indent=opts.indent.." "

			opts.done[o]=true

			local maxi=0

			for k,v in ipairs(o) do -- dump number keys in order
				fout(opts.indent)
				logstring(v,opts)
				fout(",\n")
				maxi=k -- remember top
			end

			for k,v in pairs(o) do
				if (type(k)~="number") or (k<1) or (k>maxi) or (math.floor(k)~=k) then -- skip what we already dumped
					fout(opts.indent,"[")
					logstring(k,opts)
					fout("]=")
					logstring(v,opts)
					fout(",\n")
				end
			end

			opts.indent=opts.indent:sub(1,-2)
			fout(opts.indent,"}")
			return
		end
	elseif type(o) == "nil" then
		return fout("nil")
	else
		return fout("nil--[[",type(o),"]]")
	end

end


logs.dlog_cache={}
logs.dlog=function(mode,...)
	local ts={}
	for i=1,select("#", ...) do ts[i]=tostring( select(i, ...) ) end
	local s=table.concat(ts,"\t")
	if logs.dlog_cache[mode] ~= s then
		logs.dlog_cache[mode]=s
		logs.log(mode,s)
	end
end


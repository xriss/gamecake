--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstring=require("wetgenes.string")

--[[#lua.wetgenes.csv


	local wcsv = require("wetgenes.csv")

Load and save csv, prefrably using tab sperators. 

The following need to be escaped with a \ when used in each column.

	\n for newline,
	\t for tab,
	\r for carriage return,
	\\ for backslash.

and when using commas a , must be placed inside a quoted string with a 
double "" to escape " within this string.

This is intended for "small" csv files that fit in memory so does not 
stream or try and do anything clever to reduce memory overheads.

]]
local M={ modname=(...) } ; package.loaded[M.modname]=M
local wcsv=M

--[[#lua.wetgenes.csv.doesc

Escape special chars within a csv cell.

]]
wcsv.doesc=function(s)
	local map={
		["\n"]="\\n",
		["\t"]="\\t",
		["\r"]="\\r",
		["\\"]="\\\\",
	}
	return s:gsub("([\n\t\r\\])",function(w) return map[w] or w end)
end

--[[#lua.wetgenes.csv.unesc

Unescape special chars within a csv cell.

]]
wcsv.unesc=function(s)
	local map={
		["\\n"]="\n",
		["\\t"]="\t",
		["\\r"]="\r",
		["\\\\"]="\\",
	}
	return s:gsub("(\\.)",function(w) return map[w] or w end)
end

--[[#lua.wetgenes.csv.doquote

Wrap a string in quotes and escape any " within that string using csv 
rules.

]]
wcsv.doquote=function(s)
	return "\"" .. s:gsub("(\")","\"\"") .. "\""
end

--[[#lua.wetgenes.csv.unquote

Remove quotes from a strine and unescape any "" within that string. If 
the string is not in quotes then we return it as is.

]]
wcsv.unquote=function(s)
	if s:sub(1,1)=="\"" and s:sub(-1,-1)=="\"" then
		return s:sub(2,-2):gsub("(\"\")","\"")
	else
		return s -- nothing to remove
	end
end


--[[#lua.wetgenes.csv.parse

Parse csv data from a chunk of text. Returns a simple table of lines 
where each line is a table of cells. An empty or missing string 
indicates an empty cell. The second return can be ignored or used to 
build a csv in a similar format to the one we read.

	lines,opts = wcsv.parse(text)
	lines,opts = wcsv.parse(text,opts)

Opts can be used to control how the parsing is performed pass in a 
seperator value to contol how a line is split.

	lines,opts = wcsv.parse(text,{seperator=","})

Note that we also return the seperator we used within the second return 
and will guess the right one using the first line if one is not given.

]]
wcsv.parse=function(text,opts)
	opts=opts or {}
	local lines={}
	
	opts.line_seperator=opts.line_seperator or "\n"
	
	lines=wstring.split(text,opts.line_seperator)
	
	if not lines[1] then return lines,opts end -- nothing to parse
	
	if not opts.seperator and lines[1]:find("\t") then opts.seperator="\t" end
	if not opts.seperator and lines[1]:find(",") then opts.seperator="," end
	if not opts.seperator then opts.seperator="\t" end
	
	for idx=1,#lines do
		local line=wstring.split(lines[idx],opts.seperator)
		if opts.seperator=="," then
			for i=#line,2,-1 do
				local c=line[i]
				local _,ca=string.find(c,"^\"+")
				local cb,_=string.find(c,"\"+$")
				ca=ca or 0
				cb=cb and 1+#c-cb or 0
				if cb%2==1 then
					if ca%2==1 then
						-- this cell is enclosed in "
					else
						line[i-1]=line[i-1]..","..c -- merge cells as this , should be ignored
						table.remove(line,i)
					end
				end
			end
		end
		for i=1,#line do
			local cell=line[i]
			if opts.seperator~="\t" then
				cell=wcsv.unquote(cell)
			end
			cell=wcsv.unesc(cell)
			line[i]=cell
		end
		lines[idx]=line
	end

	return lines,opts
end 


--[[#lua.wetgenes.csv.build

Build csv data into a string from a simple table of lines where each 
line is a table of cells.

	text = wcsv.build(lines)
	text = wcsv.build(lines,opts)

]]
wcsv.build=function(lines,opts)
	opts=opts or {}
	local strings={}
	
	opts.line_seperator=opts.line_seperator or "\n"
	opts.seperator=opts.seperator or "\t"

	for idx=1,#lines do
		local line=lines[idx]
		local ss={}
		for i=1,#line do
			local cell=tostring(line[i])
			if opts.seperator~="\t" then
				if cell:find("\"") or cell:find(opts.seperator) then -- only if we need to quote
					cell=wcsv.doquote(cell)
				end
			end
			cell=wcsv.doesc(cell)
			ss[i]=cell
		end
		strings[idx]=table.concat(ss,opts.seperator)
	end
	
	return table.concat(strings,opts.line_seperator)..opts.line_seperator
end

--[[#lua.wetgenes.csv.map

Use the first line to map all other lines into named keys, an empty 
string will map to nil. This will return an array of items that is 
smaller than the array of lines by at least one as we also trim 
trailing empty objects.

	items = wcsv.map(lines)
	items = wcsv.map(wcsv.parse(text))

]]
wcsv.map=function(lines)
	local items={}
	
	local head=lines[1] or {}
	for idx=2,#lines do
		local line=lines[idx]
		local it={}
		for i=1,#head do
			local key=head[i]
			local val=line[i]
			if key and val and key~="" and val~="" then
				it[key]=val
			end
		end
		items[idx-1]=it
	end

-- remove trailing empty items
	for idx=#items,1,-1 do
		if next(items[idx]) then break else items[idx]=nil end
	end
	
	return items
end

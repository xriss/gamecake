--[[#lua.wetgenes.tsv

	local wtsv = require("wetgenes.tsv")

Load and save tsv files https://pypi.org/project/linear-tsv/1.0.0/

The following need to be escaped with a \ when used in each column.

	\n for newline,
	\t for tab,
	\r for carriage return,
	\\ for backslash.
	
When loading we read the entire file and keep all the text in one chunk 
with function lookups to cut out sections of string as needed.

]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wstring=require("wetgenes.string")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local wtsv=M

local doesc=function(s)
	local map={
		["\n"]="\\n",
		["\t"]="\\t",
		["\r"]="\\r",
		["\\"]="\\\\",
	}
	return s:gsub("([\n\t\r\\])",function(w) return map[w] or w end)
end

local unesc=function(s)
	local map={
		["\\n"]="\n",
		["\\t"]="\t",
		["\\r"]="\r",
		["\\\\"]="\\",
	}
	return s:gsub("(\\.)",function(w) return map[w] or w end)
end


--[[#lua.wetgenes.tsv.create

	wtsv.create()
	wtsv.create({filename="filename.tsv"})
	wtsv.create({basedata="1\t2\t3\n4\t5\t6\n"})

Create a tsv, possibly from a datachunk or file

]]
wtsv.create=function(opts)
	opts=opts or {}

	tsv = {}
	setmetatable(tsv,wtsv) -- expose wtsv.functions as tsv:functions

	if opts.filename then
		tsv.filename=opts.filename
		tsv:load()
	elseif opts.basedata then
		tsv.basedata=opts.basedata
		tsv:parse()
	else
		tsv:parse()
	end

	return tsv
end



--[[#lua.wetgenes.tsv.load

parse a base chunk of data from tsv.basedata into tsv.lines

]]
wtsv.parse=function(tsv)

	tsv.lines={} -- this is empty 
	
	tsv.baseidxs={} -- pairs of start,end indexs of each line we find
	
	local lidx=1
	local cidx=1

	if tsv.basedata then -- have some basedata to parse

		tsv.baseidxs[lidx]=cidx
		lidx=lidx+1

		while true do
			local o=tsv.basedata:find("\n",cidx,true)
			if o then
				tsv.baseidxs[lidx]=o-1 -- end of last line
				tsv.baseidxs[lidx+1]=o+1 -- start of new line
				cidx=o+1
				lidx=lidx+2
			else
				if tsv.baseidxs[lidx-1] > #tsv.basedata then -- ignore last empty line
					tsv.baseidxs[lidx-1] = nil
				else
					tsv.baseidxs[lidx]=#tsv.basedata -- final line has no \n at end?
				end
				break
			end
		end

	end

	tsv.baselines=math.floor(#tsv.baseidxs/2) -- number of lines
	tsv.numoflines=tsv.baselines
end


--[[#lua.wetgenes.tsv.load

load a tsv file from tsv.filename

]]
wtsv.load=function(tsv)

	local fp=io.open(tsv.filename,"rb")
	tsv.basedata=fp:read("*all")
	fp:close()
	
	tsv:parse()

end


--[[#lua.wetgenes.tsv.load

save a tsv file to tsv.filename

]]
wtsv.save=function(tsv)

	local fp=io.open(tsv.filename,"wb")
	
	for i=1,tsv.numoflines do
		local line=tsv:line(i) or {""}
		for i=1,#line do
			fp:write(doesc(line[i])
			if i==#line then fp:write("\n")
			else             fp:write("\t")
			end
		end
	end

	fp:close()
end


--[[#lua.wetgenes.tsv.flush

save recent changes to disk, appended to loaded file

]]
wtsv.flush=function(tsv)

end


--[[#lua.wetgenes.tsv.close

close any files we may have open, be sure to flush first

]]
wtsv.close=function(tsv)

end


--[[#lua.wetgenes.tsv.close

get or set a line of data as a table of strings

]]
wtsv.line=function(tsv,idx,value)

	if value then -- set
		tsv.lines[idx]=value
		if idx > tsv.numoflines then tsv.numoflines = idx end
	end
	
	local it = tsv.lines[idx]

	if it then return it end -- found it
	
	if idx <= tsv.baselines then -- build it
		local bidx=(idx*2)-1
		local line=tsv.basedata:sub( tsv.baseidxs[bidx] , tsv.baseidxs[bidx+1] )
		local it={}
		local i=1
		while true do
			local e=line:find("\t",i)
			if e then
				it[#it+1]=unesc(line:sub(i,e-1))
				i=e+1
			else
				it[#it+1]=unesc(line:sub(i))
				break
			end
		end
		return it
	end

end





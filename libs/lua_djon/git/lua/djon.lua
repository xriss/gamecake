

local core=require("djon.core")


--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local djon=M

local istable=function(it)
	if type(it)=="table" then return true end
	return false
end

local isarray=function(it)
	if type(it)~="table" then return false end
	local first=next(it)
	if ( type(first)=="number" ) and ( first==1 ) then return true end -- defo starts at 1
	return false
end


--[[

merge comments in com into the data in dat.

]]
djon.merge_comments=function( dat , com )

	local out=dat
	if isarray(dat) or isarray(com) then -- output must be array
		out={dat} -- must be array
		if isarray(com) then -- copy comments
			for i=2,#com do out[i]=com[i] end
			com=com[1]
		end
	end
	if type(dat)=="table" then -- need to recurse
		local o={}
		for n,d in pairs(dat) do
			local c=istable(com) and com[n]
			o[n]=djon.merge_comments(d,c)
		end
		if isarray(out) then out[1]=o else out=o end
	end

	return out
end

--[[

Remove comments converting com back into standard json data and 
returning it.

]]
djon.remove_comments=function( com )

	local out=com
	if isarray(out) then out=out[1] end -- unescape
	if istable(out) then -- need to recurse
		local o={}
		for n,d in pairs(out) do o[n]=djon.remove_comments(d) end
		out=o
	end

	return out
end


--[[

load a json/djon file

]]
djon.load_file=function(filename,...)
	local it={}
	it.filename=filename

    local f = assert( io.open( filename, "rb") )
    it.input = f:read("*all")
    f:close()
    
	return djon.load_core(it,...)
end

--[[

load a json/djon text

]]
djon.load=function(text,...)
	local it={}
	it.input=text
	return djon.load_core(it,...)
end

--[[

load a json/djon

]]
djon.load_core=function(it,...)
	local args={...}
	it.ds=core.setup()
	local r,e=xpcall( function() core.load( it.ds , it.input , unpack(args) ) end , function(e)
		local l,c,b=core.location(it.ds)
		return( e.." line "..l.." char "..c.." byte "..b )
	end)
	if not r then
		error(e,2)
	end
	it.output=core.get(it.ds,...)
	return it.output
end


--[[

merge data with comments already in a djon file, then save over it.

This is designed to keep configuration file comments. Load file as just 
json data modify it as you wish then use this function to reapply 
comments and write back out.

]]
djon.save_comments=function(filename,tab,...)

	local com -- maybe we have comments to merge from old file
	pcall( function() com=djon.load_file(filename,"comments") end ) -- load but do not complain
	local it={}
	it.filename=filename
	it.tab=djon.merge_comments( tab , com ) -- merge comments
	return djon.save_core(it,"comments","djon",...) -- force djon and comments flag
end

--[[

save data in a json/djon file

]]
djon.save_file=function(filename,tab,...)
	local it={}
	it.filename=filename
	it.tab=tab
	return djon.save_core(it,...)
end

--[[

save data in a json/djon string

]]
djon.save=function(tab,...)
	local it={}
	it.tab=tab
	return djon.save_core(it,...)
end

--[[

save data in a json/djon format

]]
djon.save_core=function(it,...)

	it.ds=core.setup()
	core.set( it.ds , it.tab , ... )
	local opts={}
	
	it.output=core.save(it.ds,...)
	
	if it.filename then
		local fp=io.open(it.filename,"wb")
		fp:write(it.output)
		fp:close()
	else
		return it.output
	end
end




local core=require("djon.core")


--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local djon=M

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

	it.ds=core.setup()
	local r,e=xpcall( function(...) core.load( it.ds , it.input , ... ) end , function(e)
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


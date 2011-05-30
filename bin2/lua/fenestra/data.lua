

local hex=function(str) return tonumber(str,16) end

local io=io
local string=string
local table=table
local ipairs=ipairs
local math=math
local loadstring=loadstring
local pcall=pcall

-- imported global functions
local sub = string.sub
local match = string.match
local find = string.find
local push = table.insert
local pop = table.remove
local append = table.insert
local concat = table.concat
local floor = math.floor
local write = io.write
local read = io.read
local type = type
local setfenv = setfenv
local tostring=tostring
local pairs=pairs
local ipairs=ipairs
local unpack=unpack

local _G = _G

local wetlua=wetlua
local wldir=wetlua.dir or ""

module("fenestra.data")



function setup(fenestra)

	local print=fenestra._g.print
	
	local it={}
	
	function it.clean()

	end
	
	function it.load(fname)

		local fp=io.open(wldir..fname,"rb")
		local dat=fp:read("*all")
		fp:close();
		
		
		return fenestra.load(dat)

	end
	
	return it
end

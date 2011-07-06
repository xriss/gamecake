
-- shared attributes, across cells, items and chars
-- we metamap .attr in these tables so cell.get gets attributes

local _G=_G

local table=table
local pairs=pairs
local ipairs=ipairs
local string=string
local math=math
local os=os

local setfenv=setfenv
local unpack=unpack
local require=require
local type=type
local print=print

module(...)
local attrdata=require("yarn.attrdata")

-- pass in a table created by attrdata.get
function create(ad)
	
local d=ad
setfenv(1,d)

-- any state data you expect to persist must be stored in the base table
-- never change any of the sub tables, such as .can or .call these are
-- shared tables so any change will effect all other objects of the same class


	hpmax=hp -- remember initial hp

	set={}
	get={}

	function set.name(v)       name=v end
	function get.name() return name   end
	
	function set.visible(v)       visible=v end
	function get.visible() return visible   end
	
	
--	function get.visible() return true end -- debug

	return d
	
end

-- create a save state for this attr which contains enough information
-- to recreate this attr when combined with the attrdata tables
-- so this is a diff from an attrdata.get
-- the result should be good to save as json
function save(it)

	local ad=attrdata.get(it.name,it.pow) -- get base data to compare
	local sd={}
	for i,v in pairs(it) do
		if ( type(ad[i])==type(v) ) and ad[i]==v then -- no change from base
		else
			if type(v)=="table" then --ignore tables, one deep save only
			else
				sd[i]=v -- this is something we are interested in
			end
		end
	end
-- always include these two

	sd.name=it.name
	sd.pow=it.pow
	if sd.pow==0 then sd.pow=nil end
	
	return sd -- a table of changes to base data
end

-- reload a saved data (use instead of create)
function load(sd)
	return create( attrdata.get(sd.name,sd.pow,sd) ) -- unpack and create
end



-- shared attributes, across cells, items and chars
-- we metamap .attr in these tables so cell.get gets attributes

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug

module(...)
local yarn_attrs=require("yarn.attrs")

-- pass in a table created by yarn_attrs.get
function create(ad)
	
local d=ad
setfenv(1,d)

-- any state data you expect to persist must be stored in the base table
-- never change any of the sub tables, eg .can as these are
-- shared tables so any change will effect all other objects of the same class
-- all datagoes into this main table


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
-- so this is a diff from an yarn_attrs.get
-- the result should be good to save as json
function save(it)

	local ad=yarn_attrs.get(it.name,it.pow) -- get base data to compare
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
	return create( yarn_attrs.get(sd.name,sd.pow,sd) ) -- unpack and create
end


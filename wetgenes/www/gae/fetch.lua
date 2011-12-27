

local core=require("wetgenes.aelua.fetch.core")

local os=os

module("wetgenes.aelua.fetch")

function countzero()
	count=0
	api_time=0
end
countzero()

local kind_props={}	-- default global props mapped to kinds

local start_time -- handle simple api benchmarking of some calls
local function apis()
	start_time=os.time()
end
local function apie(...)
	api_time=api_time+os.time()-start_time
	return ...
end




function get(...)
	apis()
	count=count+1
	return apie(core.get(...))
end



function post(...)
	apis()
	count=count+1
	return apie(core.post(...))
end

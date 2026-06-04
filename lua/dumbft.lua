
local fats=require("wetgenes.fats")

local core=require("dumbft.core")

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local dumbft=M

--[[

	probelens is array of probelengths in full samples then wavcount for this probe
	two numbers per probe
	so wavelens are fractional -> [1]/[2] , [3]/[4] , etc
	
]]
dumbft.create=function(probelens)
	local dft={}
	dft.probelens=probelens

	dft[0]=core.setup( fats.table_to_int32s(probelens) )
	
	dft.push=function(dft,s16s)
		core.push( dft[0] , s16s )
	end

	dft.pull=function(dft)
		return core.pull(dft[0])
	end

	return dft
end

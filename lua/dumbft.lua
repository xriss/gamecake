
local fats=require("wetgenes.fats")

local core=require("dumbft.core")

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local dumbft=M


dumbft.create=function(wavlens)
	local dft={}
	dft.wavlens=wavlens

	dft[0]=core.setup( fats.table_to_int32s(wavlens) )
	
	dft.push=function(dft,s16s)
		core.push( dft[0] , s16s )
	end

	dft.pull=function(dft)
		return core.pull(dft[0])
	end

	return dft
end


module(...,package.seeall)

local wstr=require("wetgenes.string")


function test_al()

local al=require("al")
local alc=require("alc")

	print("AL",wstr.dump(al))
	print("ALC",wstr.dump(alc))
	
--	print(al.NO_ERROR,"==",al[al.NO_ERROR])

--	al.test()

end



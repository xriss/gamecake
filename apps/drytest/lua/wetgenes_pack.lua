

module(...,package.seeall)

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")


function test_alloc()

	local t=pack.alloc(64)
	
	print()
	print(tostring(t),pack.sizeof(t))

end


function test_pack()


	local s1="12341234"
	local d={"u8","bit8a","u8","bit8b","u16","worda","u32","longa",bigend=false}
	
--	print(wstr.dump(pack))

	local r=pack.load(s1,d)
	
--	print(wstr.dump(r))

	local s2=pack.save(r,d)
	
--	print("OUT",s1,s2)
	
	assert(s1==s2)

end

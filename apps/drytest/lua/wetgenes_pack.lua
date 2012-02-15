

module(...,package.seeall)

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")



function test_pack()


	local s1="12341234"
	local d={"u8","u8","u16","u32",bigend=false}
	
--	print(wstr.dump(pack))

	local r=pack.load(s1,d)
	
--	print(wstr.dump(r))

	local s2=pack.save(r,d)
	
--	print("OUT",s1,s2)
	
	assert(s1==s2)

end

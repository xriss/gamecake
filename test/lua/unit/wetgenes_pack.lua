

module(...,package.seeall)

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")


function test_alloc()

	local d=pack.alloc(64)
	local siz=pack.sizeof(d)
--print(siz)
	assert(siz==64)

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


function test_buffpack()

	local buff=pack.alloc(16*4)
	
	pack.save_array( {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16},"f32",0,16 )

	local s=pack.tostring(buff)
	
--	print(s)

	

end


module(...,package.seeall)

local tardis=require("wetgenes.tardis")

function test_m4_transpose()

	local m4a=tardis.m4.new( { 1 , 2 , 3 , 4  } , { 5 , 6 , 7  , 8  } , { 9 , 10 , 11 , 12 } , { 13 , 14 , 15 , 16 } )
	local m4b=tardis.m4.new( { 1 , 5 , 9 , 13 } , { 2 , 6 , 10 , 14 } , { 3 , 7  , 11 , 15 } , { 4  , 8  , 12 , 16 } )
	local m4c=m4a:transpose(tardis.m4.new())
	local r=m4c:compare(m4b)
	assert( r , "\n INPUT \n"..tostring(m4a).."\n RESULT \n"..tostring(m4c).. "\n EXPECT \n"..tostring(m4b) )

end


function test_m4_inverse()

	local m4a=tardis.m4.new( { 1 , 0 , 0 , 1  } , { 0 , 2   , 0 , 1    } , { 0 , 0 , 2   , 1    } , { 0 , 0 , 0 , 1 } )
	local m4b=tardis.m4.new( { 1 , 0 , 0 , -1 } , { 0 , 0.5 , 0 , -0.5 } , { 0 , 0 , 0.5 , -0.5 } , { 0 , 0 , 0 , 1 } )
	local m4c=m4a:inverse(tardis.m4.new())
	local r=m4c:compare(m4b)
	assert( r , "\n INPUT \n"..tostring(m4a).."\n RESULT \n"..tostring(m4c).. "\n EXPECT \n"..tostring(m4b) )

end



module(...,package.seeall)

local tardis=require("wetgenes.tardis")
local tardis_core=require("wetgenes.tardis.core")

function test_core()

	local V4=tardis.v4.new
	local ta=V4( 1 , 2 , 3 , 4 )
	if type(ta)=="cdata" then -- test cdata wrapper
		local tb=V4( tardis_core.test_v4_ptr(ta) ) -- test we can get cdata on c side
		assert(ta==tb)
	end

end

function test_v1_math()

	local V1=tardis.v1.new
	assert( #V1()==1 )

	local ta=V1( 1 )
	local tb=V1( 2 )
	local tc=ta+tb
	local tt=V1( 3 )
	assert( tc==tt , "\n INPUT \n"..tostring(ta).."\n"..tostring(tb).."\n RESULT \n"..tostring(tc).. "\n EXPECT \n"..tostring(tt) )

end

function test_v2_math()

	local V2=tardis.v2.new
	assert( #V2()==2 )

	local ta=V2( 1 , 2 )
	local tb=V2( 3 , 4 )
	local tc=ta+tb
	local tt=V2( 4 , 6 )
	assert( tc==tt , "\n INPUT \n"..tostring(ta).."\n"..tostring(tb).."\n RESULT \n"..tostring(tc).. "\n EXPECT \n"..tostring(tt) )

end

function test_v3_math()

	local V3=tardis.v3.new
	assert( #V3()==3 )

	local ta=V3( 1 , 2 , 3 )
	local tb=V3( 4 , 5 , 6 )
	local tc=ta+tb
	local tt=V3( 5 , 7 , 9 )
	assert( tc==tt , "\n INPUT \n"..tostring(ta).."\n"..tostring(tb).."\n RESULT \n"..tostring(tc).. "\n EXPECT \n"..tostring(tt) )

end


function test_v4_math()

	local V4=tardis.v4.new
	assert( #V4()==4 )

	local ta=V4( 1 , 2 , 3 , 4 )
	local tb=V4( 5 , 6 , 7 , 8 )
	local tc=ta+tb
	local tt=V4( 6 , 8 , 10 , 12 )
	assert( tc==tt , "\n INPUT \n"..tostring(ta).."\n"..tostring(tb).."\n RESULT \n"..tostring(tc).. "\n EXPECT \n"..tostring(tt) )

end


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


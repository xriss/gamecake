
--HAXTBH, ffi seems bad so just remove it
package.preload.ffi=nil
local lanes=require("lanes").configure(1)


module(...,package.seeall)


--
-- REQUIRE.LUA
--
-- Test that 'require' works from sublanes
--

local function a_lane()
local wstr=require("wetgenes.string")

    -- To require 'math' we still actually need to have it initialized for
    -- the lane.
    --
--print(math)
    require "math"
--print("oo2")
    assert( math and math.sqrt )
--print("oo3")
    assert( math.sqrt(4)==2 )
--print("oo4")

--    assert( lanes==nil )
--print("oo5")
    local lanes=require("lanes").configure(1)
--lanes.configure(1)
--print(wstr.dump(lanes))

    local h= lanes.gen( function() return 42 end ) ()
--print("oo8")
    local v= h[1]
--print("oo9")

    return v==42
end

function test_require()
local lanes=require("lanes")
--print("poo1")
	local gen= lanes.gen( "*", a_lane )
--print("poo2")

	local h= gen()
--print("poo3")
	local ret= h[1]
--print("poo4")
--print("ret=",ret)
	assert( ret==true )

	local gen= lanes.gen( "*", a_lane )
	local h= gen()
	local ret= h[1]

end



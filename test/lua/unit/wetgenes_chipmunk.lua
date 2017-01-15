

module(...,package.seeall)

local wstr=require("wetgenes.string")
local chipmunk=require("wetgenes.chipmunk")



function test_space()

	local space=chipmunk.space()
	
	space:gravity(0,700)
	space:damping(0.5)
	space:sleep_time_threshold(1)
	space:idle_speed_threshold(10)

	for i=1,120 do
		space:step(1/60)
	end

end

function test_body()

	local space=chipmunk.space()

	local body=space:body(0.1,0.1)
	body:position(0,0)
	body:velocity(1,1)

	local shape=body:shape("circle",1,0,0)

	for i=1,2*60 do
		space:step(1/60)
	end
	
	local px,py=body:position() -- where we should be now

	assert(math.floor(px*1000+0.5)==2000) -- check to a few decimal places
	assert(math.floor(py*1000+0.5)==2000)

end


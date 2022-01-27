

module(...,package.seeall)

local wstr=require("wetgenes.string")
local chipmunk=require("wetgenes.chipmunk")
local ls=function(...)print(wstr.dump(...)) end


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


local function space_setup()

	local space=chipmunk.space()

	local bodys={}
	local shapes={}
	local function ball(x,y,r)
		local body=space:body(0.1,0.1)
		local shape=body:shape("circle",r,x,y)
		bodys[#bodys]=body
		shapes[#shapes]=shape
		return shape,body
	end
	
	ball(-10,0,3)
	ball( -5,0,2)
	ball(  0,0,1)
	ball(  5,0,2)
	ball( 10,0,3)
	ball(0,-10,3)
	ball(0, -5,2)
	ball(0,  5,2)
	ball(0, 10,3)
	
	return space,bodys,shapes

end

function test_query_point()

	local space,bodys,shapes=space_setup()
	
	local ret=space:query_point(0,0,10,0,0xffffffff,0xffffffff)
	assert(#ret==9)

	local ret=space:query_point(1,0,1,0,0xffffffff,0xffffffff)
	assert(#ret==1)

	local ret=space:query_point(0,25,20,0,0xffffffff,0xffffffff)
	assert(#ret==2)

	local ret=space:query_point(5,5,4,0,0xffffffff,0xffffffff)
	assert(#ret==2)

	local ret=space:query_point(10,10,2,0,0xffffffff,0xffffffff)
	assert(#ret==0)

end

function test_query_point_nearest()

	local space,bodys,shapes=space_setup()
		
	local ret=space:query_point_nearest(10,10,2,0,0xffffffff,0xffffffff)
	assert(ret==nil)

	local ret=space:query_point_nearest(10,10,100,0,0xffffffff,0xffffffff)
	assert(ret.point_x==10)
	assert(ret.point_y==3)

end

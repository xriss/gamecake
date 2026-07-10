

local box2d=require("box2d")

-- important simulation values
local METER=16
local SQR_METER=METER*METER

local var_tostring=function(v)
	if type(v)=="table" then -- any tables will be arrays of numbers
		if v.is then
			local a={}
			for n,v in pairs(v) do a[#a+1]=n.."="..tostring(v) end
			return "{ "..table.concat(a," , ").." }"
		else
			return "{ "..table.concat(v," , ").." }"
		end
	else
		return tostring(v)
	end
end

-- adjust default box2d variables
box2d.set({
	LengthUnitsPerMeter=METER,
})

print("box2d")
for n,v in pairs( box2d:get() ) do
	print("box2d",n,"=",var_tostring(v))
end

-- create a world
local world=box2d.world({
	gravity={0,10*METER,0},
})
print("world")
for n,v in pairs( world:get() ) do
	print("world",n,"=",var_tostring(v))
end

-- create body
local bodyA=world:body({
})
print("bodyA")
for n,v in pairs( bodyA:get() ) do
	print("body",n,"=",var_tostring(v))
end

local shapeA=bodyA:shape({
	shape="circle",
	radius=1*METER,
})
print("shapeA")
for n,v in pairs( shapeA:get() ) do
	print("shape",n,"=",var_tostring(v))
end

-- create body
local bodyB=world:body({
})
print("bodyB")
for n,v in pairs( bodyB:get() ) do
	print("body",n,"=",var_tostring(v))
end

local shapeB=bodyB:shape({
	shape="circle",
	radius=1*METER,
})
print("shapeB")
for n,v in pairs( shapeB:get() ) do
	print("shape",n,"=",var_tostring(v))
end

local joint=world:joint({
	joint="distance",
	bodyA=bodyA,
	localFrameA={0,0,0},
	bodyB=bodyB,
	localFrameB={0,0,0},
	forceThreshold=0,
})
print("joint")
for n,v in pairs( joint:get() ) do
	print("joint",n,"=",var_tostring(v))
end

local hits=world:cast_ray({
	origin_y=-100*METER,
	translation_y=100*METER,
})
print("raycast")
for i,hit in ipairs(hits) do
	for n,v in pairs( hit ) do
		print("hit",i,n,"=",var_tostring(v))
	end
end


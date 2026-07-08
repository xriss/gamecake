

local box2d=require("box2d")

-- important simulation values
local METER=16
local SQR_METER=METER*METER

local var_tostring=function(v)
	if type(v)=="table" then -- any tables will be arrays of numbers
		return "{ "..table.concat(v," , ").." }"
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
local body=world:body({
})

print("body")
for n,v in pairs( body:get() ) do
	print("body",n,"=",var_tostring(v))
end
